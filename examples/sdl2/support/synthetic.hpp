#pragma once
#include <mig/core/engine.hpp>
#include <mig/hands/fingers.hpp>
namespace demo {
inline void synthetic_frame(std::uint64_t sequence, mig::Frame& body, mig::hands::Frame& hands) {
    // Explicit synthetic data for learning/testing, never presented as detection.
    const auto t = std::int64_t(sequence * 20);
    body = {};
    body.timestamp_ms = t;
    body.sequence = sequence;
    body.aspect = 1;
    body.points[11] = {{.65f, .45f}, 1};
    body.points[12] = {{.35f, .45f}, 1};
    mig::Grid grid;
    grid.center = {.5f, .45f};
    grid.scale = .06f;
    const float row = t < 1200 ? 5.5f : std::max(1.5f, 5.5f - float(sequence - 59) / 5);
    body.points[15] = {grid.metric({6.5f, row}), 1};
    body.points[16] = {grid.metric({2.5f, row}), 1};
    for (auto& point : body.points) {
        point.depth_valid = point.confidence > 0;
    }
    hands = {};
    hands.timestamp_ms = t;
    hands.sequence = sequence;
    hands.aspect = 1;
    hands.count = 1;
    auto& hand = hands.hands[0];
    const auto wrist = body.points[15].position;
    hand.points[0] = {wrist.x, wrist.y, 0};
    for (int finger = 0; finger < 5; ++finger) {
        for (int joint = 0; joint < 4; ++joint) {
            hand.points[1 + finger * 4 + joint] = {wrist.x + (finger - 2) * .015f,
                                                   wrist.y - (joint + 1) * .018f, -.005f * joint};
        }
    }
    const auto observations = mig::hands::hand_observations(hands, body);
    body.fingers = observations.fingers;
    body.hand_contacts = observations.contacts;
}
} // namespace demo
