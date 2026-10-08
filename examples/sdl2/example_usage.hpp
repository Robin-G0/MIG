#pragma once
#include "support/profile.hpp"
#include <iostream>
#include <mig/core/engine.hpp>
namespace tutorial {
inline mig::Configuration load_configuration(const demo::Options& options) {
    // Parse and validate the JSON once, before processing observations.
    auto configuration = mig::load_configuration(options.config);
    if (demo::profile_mode && !options.smoke) {
        configuration.motions.clear();
        configuration.track_hands = false;
    }
    return configuration;
}

inline mig::Engine initialize_mig(const demo::Options& options) {
    // Engine owns calibration and movement state. Its destructor releases them.
    return mig::Engine(load_configuration(options));
}

inline unsigned process_tracking_frame(mig::Engine& engine, const mig::Frame& body, std::string* status = nullptr,
                        bool hand_messages = true) {
    // Submit one fresh, unmirrored observation. The returned span is borrowed
    // until the next update/reset, so consume its action IDs immediately.
    const auto events = engine.update(body, body.timestamp_ms);
    if (!events.empty() && status) {
        status->clear();
    }
    for (const auto& event : events) {
        // Bind logical actions directly to your game. This never injects OS keys.
        const auto& motion = engine.configuration().motions[event.motion];
        const auto& action = motion.action;
        const auto message = hand_messages && action == "left_raise" ? "Left hand raised!"
                             : hand_messages && action == "right_raise"
                                 ? "Right hand raised!"
                                 : "Action: " + action + " (input " + motion.id + ")";
        std::cout << message << std::endl;
        if (status) {
            if (!status->empty()) {
                *status += " | ";
            }
            *status += message;
        }
    }
    return unsigned(events.size());
}
} // namespace tutorial
