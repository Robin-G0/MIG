#pragma once
#include <iostream>
#include <mig/core/engine.hpp>
#include <mig/format/configuration.hpp>
inline void calibrate(mig::Engine& engine, mig::Frame& frame) {
    // MIG calibrates its grid from stable shoulder observations over time.
    frame.aspect = 1;
    frame.points[11] = {{0.65f, 0.45f}, 1};
    frame.points[12] = {{0.35f, 0.45f}, 1};
    for (int sample = 1; sample <= 25; ++sample) {
        frame.timestamp_ms = sample * 50;
        frame.sequence = sample;
        engine.update(frame, frame.timestamp_ms);
    }
}
inline void dispatch_events(const mig::Engine& engine, std::span<const mig::Event> events) {
    // Events are borrowed until the next update; bind each action to your game now.
    for (const auto& event : events) {
        std::cout << "Game event: " << engine.configuration().motions[event.motion].action << '\n';
    }
}
inline void demonstrate_path(mig::Engine& engine, mig::Frame& frame) {
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
inline mig::Engine initialize_mig(const std::string& configuration_path) {
    // load_configuration(path) validates JSON. Engine owns the resulting rules
    // and temporal state; scope exit cleans up without a separate shutdown call.
    return mig::Engine(mig::load_configuration(configuration_path));
}
