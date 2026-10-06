#pragma once
#include <cmath>
#include <mig/core/engine.hpp>
#include <optional>

namespace mig {
enum class CoordinateSystem { Image, World, WorldHeightUp };
struct Coordinate3D {
    float x{}, y{}, z{}, confidence{};
};
// O(1), no allocation. Borrow the raw span for bulk processing; resolve names
// with landmark_index once at setup, rather than in the per-frame loop.
inline std::span<const Point, 33> body_coordinates(const Frame& frame) noexcept {
    return frame.points;
}
inline std::optional<Coordinate3D>
body_coordinate(const Frame& frame, int landmark, CoordinateSystem system = CoordinateSystem::Image,
                float minimum_confidence = .6f) noexcept {
    if (landmark < 0 || landmark >= 34 || !std::isfinite(minimum_confidence) ||
        minimum_confidence < 0 || minimum_confidence > 1) {
        return std::nullopt;
    }
    const auto p = body_point(frame, landmark);
    if (!std::isfinite(p.confidence) || p.confidence < minimum_confidence || p.confidence <= 0 ||
        p.confidence > 1) {
        return std::nullopt;
    }
    Coordinate3D result;
    if (system == CoordinateSystem::Image) {
        if (!p.depth_valid || p.position.x < 0 || p.position.x > 1 || p.position.y < 0 ||
            p.position.y > 1) {
            return std::nullopt;
        }
        result = {p.position.x, p.position.y, p.depth, p.confidence};
    } else if (system == CoordinateSystem::World || system == CoordinateSystem::WorldHeightUp) {
        if (!p.world_valid) {
            return std::nullopt;
        }
        result = {p.world[0], system == CoordinateSystem::WorldHeightUp ? -p.world[1] : p.world[1],
                  p.world[2], p.confidence};
    } else {
        return std::nullopt;
    }
    if (!std::isfinite(result.x) || !std::isfinite(result.y) || !std::isfinite(result.z)) {
        return std::nullopt;
    }
    return result;
}
} // namespace mig
