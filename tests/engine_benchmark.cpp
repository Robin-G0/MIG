#include <chrono>
#include <iostream>
#include <mig/core/engine.hpp>

namespace {
mig::Frame frame(std::int64_t time) {
    mig::Frame result;
    result.timestamp_ms = time;
    result.sequence = static_cast<std::uint64_t>(time);
    result.aspect = 1;
    result.points[11] = {{0.65f, 0.45f}, 1};
    result.points[12] = {{0.35f, 0.45f}, 1};
    return result;
}
void measure(std::size_t motions, std::size_t points) {
    mig::Configuration profile;
    for (std::size_t i = 0; i < motions; ++i) {
        mig::Motion motion;
        motion.id = "motion_" + std::to_string(i);
        mig::Step step{"route", mig::StepMode::Ordered};
        for (std::size_t j = 0; j < points; ++j) {
            step.constraints.push_back(
                {"cell_" + std::to_string(j),
                 15,
                 {4, 5 - int(j % 4)},
                 j + 1 == points ? mig::ConstraintType::Trigger : mig::ConstraintType::Required});
        }
        motion.steps = {std::move(step)};
        profile.motions.push_back(std::move(motion));
    }
    mig::Engine engine(std::move(profile));
    for (int i = 1; i <= 25; ++i) {
        const auto pose = frame(i * 50);
        engine.update(pose, pose.timestamp_ms);
    }
    constexpr int updates = 100000;
    const auto start = std::chrono::steady_clock::now();
    std::size_t events = 0;
    for (int i = 0; i < updates; ++i) {
        auto pose = frame(1300 + std::int64_t(i) * 30);
        const float y = 5.5f - float(i % 4);
        pose.points[15] = {engine.grid().metric({4.5f, y}), 1};
        events += engine.update(pose, pose.timestamp_ms).size();
    }
    const auto elapsed =
        std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start).count();
    std::cout << "motions=" << motions << " points=" << points << " ns/update=" << elapsed / updates
              << " events=" << events << '\n';
}
} // namespace
int main() {
    for (const std::size_t motions : {1u, 16u, 64u}) {
        for (const std::size_t points : {2u, 64u}) {
            measure(motions, points);
        }
    }
}
