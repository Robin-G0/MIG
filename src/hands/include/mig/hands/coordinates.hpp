#pragma once
#include <mig/core/coordinates.hpp>
#include <mig/hands/frame.hpp>
namespace mig::hands {
inline std::span<const Position, 21> hand_coordinates(const Hand& hand) noexcept {
    return hand.points;
}
// The host has already selected a present hand from Frame::hands[0,count).
// Handedness classification score is deliberately not reported as point confidence.
inline std::optional<Position>
hand_coordinate(const Hand& hand, Landmark landmark,
                CoordinateSystem system = CoordinateSystem::Image) noexcept {
    const auto index = std::size_t(landmark);
    if (index >= 21 || (system != CoordinateSystem::Image && !hand.world_valid)) {
        return std::nullopt;
    }
    auto point = system == CoordinateSystem::Image ? hand.points[index] : hand.world_points[index];
    if (system == CoordinateSystem::WorldHeightUp) {
        point.y = -point.y;
    }
    if ((system != CoordinateSystem::Image && system != CoordinateSystem::World &&
         system != CoordinateSystem::WorldHeightUp) ||
        !std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z)) {
        return std::nullopt;
    }
    return point;
}
} // namespace mig::hands
