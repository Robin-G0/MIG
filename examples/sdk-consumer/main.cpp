#include <iostream>
#include <mig/core/engine.hpp>
#include <mig/format/configuration.hpp>
void calibrate(mig::Engine& engine, mig::Frame& frame) {
    frame.aspect = 1;
    frame.points[11] = {{0.65f, 0.45f}, 1};
    frame.points[12] = {{0.35f, 0.45f}, 1};
    for (int sample = 1; sample <= 25; ++sample) {
        frame.timestamp_ms = sample * 50;
        frame.sequence = sample;
        engine.update(frame, frame.timestamp_ms);
    }
}
void dispatch_events(const mig::Engine& engine, std::span<const mig::Event> events) {
    for (const auto& event : events) {
        std::cout << "Game event: " << engine.configuration().motions[event.motion].action << '\n';
    }
}
void demonstrate_path(mig::Engine& engine, mig::Frame& frame) {
    if (engine.configuration().motions.empty()) {
        throw std::runtime_error("This demonstration needs an input with an authored path");
    }
    // An intentionally simplified synthetic walk, not a general profile executor:
    // complex fingers, simultaneous lanes and Interaction need real observations.
    for (const auto& step : engine.configuration().motions[0].steps) {
        for (const auto& constraint : step.constraints) {
            const auto& cell = constraint.cell;
            const int landmark = constraint.landmark == 33 ? 0 : constraint.landmark;
            frame.points[landmark] = {engine.reference_grid().metric(
                                          {cell.x + cell.width * .5f, cell.y + cell.height * .5f}),
                                      1};
            frame.timestamp_ms += 50;
            ++frame.sequence;
            dispatch_events(engine, engine.update(frame, frame.timestamp_ms));
        }
    }
}
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: mig-sdk-example config.json\n";
        return 1;
    }
    try {
        mig::Engine engine(mig::load_configuration(argv[1]));
        mig::Frame frame;
        calibrate(engine, frame);
        demonstrate_path(engine, frame);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
