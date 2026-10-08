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

inline void handle_detected_actions(const mig::Engine& engine, std::span<const mig::Event> events,
                                    std::string* status, bool hand_messages) {
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
}

inline unsigned process_tracking_frame(mig::Engine& engine, const mig::Frame& tracking_frame,
                                       std::string* status = nullptr, bool hand_messages = true) {
    // Give MIG one fresh, unmirrored observation and its monotonic time.
    // The event span is borrowed until the next update/reset, so dispatch now.
    const auto detected_actions = engine.update(tracking_frame, tracking_frame.timestamp_ms);
    handle_detected_actions(engine, detected_actions, status, hand_messages);
    return unsigned(detected_actions.size());
}
} // namespace tutorial
