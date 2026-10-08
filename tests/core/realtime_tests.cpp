#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mig/core/engine.hpp>
#include <new>

namespace {
bool measuring{};
std::size_t allocations{};

mig::Motion motion(std::size_t index) {
    mig::Motion result;
    result.id = "input_" + std::to_string(index);
    result.mirror = index % 3 != 0;
    result.space = index % 2 ? mig::CoordinateSpace::Body : mig::CoordinateSpace::Calibrated;
    const int landmark = index % 4 == 0 ? 33 : 15;
    result.steps = {
        {"route",
         mig::StepMode::Ordered,
         {{"start", landmark, {4, 5}}, {"end", landmark, {4, 4}, mig::ConstraintType::Trigger}}}};
    return result;
}

mig::Frame frame(int index) {
    mig::Frame result;
    result.timestamp_ms = index * std::int64_t(50);
    result.sequence = index;
    result.aspect = 1;
    const float shift = index < 30 ? 0.f : float(index % 7) * .003f;
    result.points[11] = {{.65f + shift, .45f}, 1};
    result.points[12] = {{.35f + shift, .45f}, 1};
    return result;
}

bool same_progress(const mig::InputProgress& a, const mig::InputProgress& b) {
    return a.constraints == b.constraints && a.interaction_elapsed_ms == b.interaction_elapsed_ms &&
           a.step == b.step && a.active == b.active && a.failed == b.failed &&
           a.triggered == b.triggered && a.fingers_valid == b.fingers_valid &&
           a.output_active == b.output_active;
}
} // namespace

void* operator new(std::size_t size) {
    if (measuring) {
        ++allocations;
    }
    if (void* memory = std::malloc(size ? size : 1)) {
        return memory;
    }
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) {
    return ::operator new(size);
}
void operator delete(void* memory) noexcept {
    std::free(memory);
}
void operator delete[](void* memory) noexcept {
    std::free(memory);
}
void operator delete(void* memory, std::size_t) noexcept {
    std::free(memory);
}
void operator delete[](void* memory, std::size_t) noexcept {
    std::free(memory);
}

int main() {
    mig::Configuration profile;
    std::array<std::unique_ptr<mig::Engine>, 64> isolated;
    for (std::size_t index = 0; index < isolated.size(); ++index) {
        profile.motions.push_back(motion(index));
        isolated[index] =
            std::make_unique<mig::Engine>(mig::Configuration{{profile.motions.back()}});
    }
    mig::Engine shared(std::move(profile));
    bool equivalent = true;
    std::size_t accepted = 0;
    for (int index = 1; index < 500; ++index) {
        auto observation = frame(index);
        if (shared.grid().valid) {
            const auto wrist = shared.grid().metric({4.5f, index % 4 < 2 ? 5.5f : 4.5f});
            observation.points[15] = observation.points[7] = observation.points[8] = {wrist, 1};
            observation.points[16] = {{1 - wrist.x, wrist.y}, 1};
            if (index % 19 == 0) {
                observation.points[15].confidence = 0;
            }
        }
        const auto now = observation.timestamp_ms + (index % 53 == 0 ? 300 : 0);
        measuring = true;
        const auto events = shared.update(observation, now);
        std::array<bool, 64> triggered{};
        for (const auto& event : events) {
            triggered[event.motion] = true;
            ++accepted;
        }
        for (std::size_t input = 0; input < isolated.size(); ++input) {
            auto& engine = *isolated[input];
            equivalent &= !engine.update(observation, now).empty() == triggered[input];
            equivalent &= same_progress(shared.progress(input), engine.progress(0));
            equivalent &= same_progress(shared.progress(input, true), engine.progress(0, true));
            equivalent &= shared.action_active(input) == engine.action_active(0);
        }
        measuring = false;
    }
    if (!equivalent || allocations != 0 || accepted == 0) {
        std::cerr << "equivalent=" << equivalent << " allocations=" << allocations
                  << " accepted=" << accepted << '\n';
        return 1;
    }
    std::cout << "Mixed-space/mirrored inputs match isolated engines without update allocations.\n";
}
