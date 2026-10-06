#include "observations.hpp"
#include <limits>
#include <mig/core/coordinates.hpp>

int main() {
    using namespace mig::native::detail;
    PoseLandmarkerResult body{};
    auto empty = body_observations(body, 12, 7, 1.5f);
    if (empty.timestamp_ms != 12 || empty.sequence != 7 || empty.points[11].confidence != 0) {
        return 1;
    }
    std::array<NormalizedLandmark, 33> body_points{};
    NormalizedLandmarks body_list{body_points.data(), 33};
    body.pose_landmarks = &body_list;
    body.pose_landmarks_count = 1;
    if (body_observations(body, 12, 7, 1.5f).points[11].confidence != 1) {
        return 2;
    }
    body_list.landmarks_count = 32;
    if (body_observations(body, 12, 7, 1.5f).points[11].confidence != 0) {
        return 3;
    }
    body_list.landmarks_count = 33;
    body_points[11].x = std::numeric_limits<float>::quiet_NaN();
    if (body_observations(body, 12, 7, 1.5f).points[11].confidence != 0) {
        return 8;
    }
    body_points[11] = {};
    body_points[11].z = -.25f;
    std::array<Landmark, 33> world_points{};
    world_points[11].x = .1f;
    world_points[11].y = -.2f;
    world_points[11].z = .3f;
    Landmarks world_list{world_points.data(), 33};
    body.pose_world_landmarks = &world_list;
    body.pose_world_landmarks_count = 1;
    const auto sample = body_observations(body, 12, 7, 1.5f);
    const auto xyz = mig::body_coordinate(sample, 11);
    const auto metric = mig::body_coordinate(sample, 11, mig::CoordinateSystem::WorldHeightUp);
    if (!xyz || xyz->z != -.25f || !metric || metric->y != .2f || metric->z != .3f) {
        return 10;
    }
    world_points[11].z = std::numeric_limits<float>::quiet_NaN();
    if (body_observations(body, 12, 7, 1.5f).points[11].world_valid) {
        return 11;
    }
    body_points[11].has_presence = true;
    body_points[11].presence = std::numeric_limits<float>::quiet_NaN();
    if (body_observations(body, 12, 7, 1.5f).points[11].confidence != 0) {
        return 9;
    }
#ifdef MIG_NATIVE_HANDS
    HandLandmarkerResult result{};
    std::array<NormalizedLandmark, 21> points{};
    std::array<NormalizedLandmarks, 2> lists{{{points.data(), 21}, {points.data(), 21}}};
    result.hand_landmarks = lists.data();
    result.hand_landmarks_count = 2;
    auto hands = hand_observations(result, 12, 7, 1.5f);
    if (hands.count != 2 || hands.timestamp_ms != 12 || hands.sequence != 7 ||
        hands.aspect != 1.5f) {
        return 4;
    }
    lists[0].landmarks_count = 20;
    if (hand_observations(result, 12, 7, 1.5f).count != 1) {
        return 5;
    }
    points[8].x = std::numeric_limits<float>::quiet_NaN();
    if (hand_observations(result, 12, 7, 1.5f).count != 0) {
        return 6;
    }
    result = {};
    if (hand_observations(result, 13, 8, 1.5f).count != 0) {
        return 7;
    }
#endif
}
