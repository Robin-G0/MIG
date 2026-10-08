#include <iostream>
#include <mig/format/configuration.hpp>
#include <stdexcept>

namespace {
mig::Frame sample(std::int64_t time) {
    mig::Frame frame;
    frame.aspect = 1;
    frame.timestamp_ms = time;
    frame.sequence = std::uint64_t(time);
    frame.points[11] = {{.65f, .45f}, 1};
    frame.points[12] = {{.35f, .45f}, 1};
    return frame;
}
unsigned raise(const mig::Configuration& config, int gap, float start, float finish) {
    mig::Engine engine(config);
    for (int time = 50; time <= 1250; time += 50) {
        engine.update(sample(time), time);
    }
    engine.restart();
    auto frame = sample(1300);
    for (const auto [joint, column] : {std::pair{15, 6.5f}, std::pair{16, 2.5f}}) {
        frame.points[joint] = {engine.grid().metric({column, start}), 1};
    }
    unsigned actions = unsigned(engine.update(frame, frame.timestamp_ms).size());
    frame.timestamp_ms += gap;
    ++frame.sequence;
    for (const auto [joint, column] : {std::pair{15, 6.5f}, std::pair{16, 2.5f}}) {
        frame.points[joint] = {engine.grid().metric({column, finish}), 1};
    }
    actions += unsigned(engine.update(frame, frame.timestamp_ms).size());
    frame.timestamp_ms += 30;
    ++frame.sequence;
    actions += unsigned(engine.update(frame, frame.timestamp_ms).size());
    return actions;
}
} // namespace
int main(int argc, char** argv) {
    try {
        if (argc != 2) {
            return 1;
        }
        const auto config = mig::load_configuration(argv[1]);
        for (const auto& motion : config.motions) {
            if (motion.steps.size() != 1 || motion.steps[0].constraints.size() != 2) {
                throw std::runtime_error("Raised-hands demo must have two broad regions");
            }
        }
        for (const auto gap : {33, 100, 160, 180}) {
            if (raise(config, gap, 5.5f, 1.5f) != 2) {
                throw std::runtime_error(
                    "Both wrists must trigger with skipped intermediate samples");
            }
        }
        if (raise(config, 181, 5.5f, 1.5f) != 0 || raise(config, 100, 1.5f, 1.5f) != 0 ||
            raise(config, 100, 1.5f, 5.5f) != 0) {
            throw std::runtime_error(
                "Gaps, isolated terminal poses and reversed paths cannot fire");
        }
        std::cout
            << "Broad demo regions: skipped samples, direction, rearm and continuity passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
