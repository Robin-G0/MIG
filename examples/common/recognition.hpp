#pragma once
#include <iostream>
#include <mig/core/engine.hpp>
namespace demo {
inline unsigned consume(mig::Engine& engine, const mig::Frame& body, std::string* status = nullptr,
                        bool hand_messages = true) {
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
} // namespace demo
