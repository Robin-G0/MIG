#include "observations.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
namespace mig::native::detail {
Frame body_observations(const PoseLandmarkerResult& result, std::int64_t t, std::uint64_t sequence,
                        float aspect) noexcept {
    Frame frame;
    frame.timestamp_ms = t;
    frame.sequence = sequence;
    frame.aspect = aspect;
    if (result.pose_landmarks_count && result.pose_landmarks &&
        result.pose_landmarks[0].landmarks) {
        const auto& list = result.pose_landmarks[0];
        for (std::size_t i = 0; i < 33 && list.landmarks_count == 33; ++i) {
            const auto& p = list.landmarks[i];
            const auto probability = [](float value) {
                return std::isfinite(value) && value >= 0 && value <= 1;
            };
            if (!std::isfinite(p.x) || !std::isfinite(p.y) || p.x < 0 || p.x > 1 || p.y < 0 ||
                p.y > 1 || (p.has_visibility && !probability(p.visibility)) ||
                (p.has_presence && !probability(p.presence))) {
                continue; // Invalid model output must never reach float-to-pixel casts.
            }
            const float confidence =
                std::min(p.has_visibility ? p.visibility : 1.f, p.has_presence ? p.presence : 1.f);
            frame.points[i] = {{p.x, p.y}, confidence};
            auto& point = frame.points[i];
            point.depth = std::isfinite(p.z) ? p.z : 0;
            point.depth_valid = std::isfinite(p.z);
            if (result.pose_world_landmarks_count && result.pose_world_landmarks &&
                result.pose_world_landmarks[0].landmarks_count == 33 &&
                result.pose_world_landmarks[0].landmarks) {
                const auto& world = result.pose_world_landmarks[0].landmarks[i];
                if (std::isfinite(world.x) && std::isfinite(world.y) && std::isfinite(world.z)) {
                    point.world = {world.x, world.y, world.z};
                    point.world_valid = true;
                }
            }
        }
    }
    return frame;
}
#ifdef MIG_NATIVE_HANDS
hands::Frame hand_observations(const HandLandmarkerResult& hand_result, std::int64_t t,
                               std::uint64_t sequence, float aspect) noexcept {
    hands::Frame output;
    output.timestamp_ms = t;
    output.sequence = sequence;
    output.aspect = aspect;
    if (hand_result.hand_landmarks) {
        for (std::size_t i = 0; i < std::min<std::size_t>(2, hand_result.hand_landmarks_count);
             ++i) {
            const auto& list = hand_result.hand_landmarks[i];
            if (!list.landmarks || list.landmarks_count != 21) {
                continue;
            }
            hands::Hand hand{};
            for (std::size_t j = 0; j < 21; ++j) {
                const auto& p = list.landmarks[j];
                hand.points[j] = {p.x, p.y, p.z};
            }
            if (hand_result.hand_world_landmarks && i < hand_result.hand_world_landmarks_count) {
                const auto& world = hand_result.hand_world_landmarks[i];
                if (world.landmarks && world.landmarks_count == 21) {
                    hand.world_valid = true;
                    for (std::size_t j = 0; j < 21; ++j) {
                        const auto& p = world.landmarks[j];
                        hand.world_points[j] = {p.x, p.y, p.z};
                        hand.world_valid = hand.world_valid && std::isfinite(p.x) &&
                                           std::isfinite(p.y) && std::isfinite(p.z);
                    }
                    if (!hand.world_valid) {
                        hand.world_points = {};
                    }
                }
            }
            if (hand_result.handedness && i < hand_result.handedness_count) {
                const auto& categories = hand_result.handedness[i];
                if (categories.categories && categories.categories_count) {
                    const auto& category = categories.categories[0];
                    hand.handedness_score = category.score;
                    if (category.category_name) {
                        if (std::strcmp(category.category_name, "Left") == 0) {
                            hand.model_side = hands::ModelSide::left;
                        } else if (std::strcmp(category.category_name, "Right") == 0) {
                            hand.model_side = hands::ModelSide::right;
                        }
                    }
                }
            }
            if (hands::valid(hand)) {
                output.hands[output.count++] = hand;
            }
        }
    }
    return output;
}
#endif
} // namespace mig::native::detail
