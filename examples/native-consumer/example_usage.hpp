#pragma once
#include <iostream>
#include <mig/native/pose.hpp>
#include <vector>
inline void demonstrate_inference(const std::string& runtime, bool hands) {
    // Pose(runtime_directory) loads MediaPipe and the bundled Full model.
    mig::native::Pose estimator(runtime);
    estimator.set_hands_enabled(hands);
    // Blank RGB tests ownership/inference only. It cannot establish human accuracy.
    std::vector<std::uint8_t> rgb(640 * 480 * 3);
    // infer(rgb, width, height, timestamp_ms, sequence) returns unmirrored
    // observations. Submit these to Engine::update() to recognize configured actions.
    const auto frame = estimator.infer(rgb, 640, 480, 1, 1);
    std::cout << "Game inference: sequence=" << frame.sequence
              << " hands-enabled=" << estimator.hands_enabled() << '\n';
    estimator.set_hands_enabled(false);
    // Scope exit destroys inference tasks before unloading the native library.
}

