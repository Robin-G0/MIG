#include <algorithm>
#include <cmath>
#include <mig/hands/fingers.hpp>
namespace mig::hands {
namespace {
float extension(const Hand& hand, int base, float aspect) {
    const auto a = hand.points[base], b = hand.points[base + 1], c = hand.points[base + 2];
    const float ax = (a.x - b.x) * aspect, ay = a.y - b.y, az = (a.z - b.z) * aspect;
    const float bx = (c.x - b.x) * aspect, by = c.y - b.y, bz = (c.z - b.z) * aspect;
    const float norm = std::sqrt((ax * ax + ay * ay + az * az) * (bx * bx + by * by + bz * bz));
    if (norm < 1e-7f) {
        return -1;
    }
    return std::clamp((-(ax * bx + ay * by + az * bz) / norm - 0.2f) / 0.7f, 0.f, 1.f);
}
} // namespace
HandObservations hand_observations(const Frame& hands, const mig::Frame& body) noexcept {
    HandObservations result{};
    auto& output = result.fingers;
    if (hands.timestamp_ms != body.timestamp_ms || hands.sequence != body.sequence ||
        hands.count > 2 || !std::isfinite(body.aspect) || body.aspect <= 0) {
        return result;
    }
    const float width =
        distance({body.points[11].position.x * body.aspect, body.points[11].position.y},
                 {body.points[12].position.x * body.aspect, body.points[12].position.y});
    if (width < 0.08f) {
        return result;
    }
    std::array<bool, 2> assigned{};
    for (std::size_t index = 0; index < hands.count; ++index) {
        const auto& hand = hands.hands[index];
        if (!valid(hand)) {
            continue;
        }
        const auto wrist = hand.points[0];
        std::array<float, 2> distances{};
        for (int side = 0; side < 2; ++side) {
            const auto point = body.points[15 + side];
            distances[side] = point.confidence >= 0.6f
                                  ? distance({wrist.x * body.aspect, wrist.y},
                                             {point.position.x * body.aspect, point.position.y})
                                  : 100.f;
        }
        const int side = distances[0] < distances[1] ? 0 : 1;
        if (distances[side] > width * 0.6f ||
            std::abs(distances[0] - distances[1]) < width * 0.15f) {
            continue;
        }
        if (assigned[side]) {
            output[side] = {};
            result.contacts[side] = {};
            result.hand_indices[side] = -1;
            continue;
        }
        assigned[side] = true;
        result.hand_indices[side] = int(index);
        const auto separation = [&](int a, int b) {
            const auto p = hand.points[a], q = hand.points[b];
            const auto dx = (p.x - q.x) * body.aspect, dy = p.y - q.y,
                       dz = (p.z - q.z) * body.aspect;
            return std::sqrt(dx * dx + dy * dy + dz * dz);
        };
        const auto palm = separation(5, 17);
        if (palm >= .005f) {
            result.contacts[side] = {separation(4, 8) / palm, 1};
        }
        for (int finger = 0; finger < 5; ++finger) {
            const int base = finger == 0 ? 1 : 5 + (finger - 1) * 4;
            const float proximal = extension(hand, base, body.aspect);
            const float distal = extension(hand, base + 1, body.aspect);
            if (proximal >= 0 && distal >= 0) {
                // A folded thumb MCP or finger DIP must not read as extended
                // merely because the other joint is straight.
                output[side][finger] = {std::min(proximal, distal), 1};
            }
        }
    }
    return result;
}
FingerObservations finger_observations(const Frame& hands, const mig::Frame& body) noexcept {
    return hand_observations(hands, body).fingers;
}
} // namespace mig::hands
