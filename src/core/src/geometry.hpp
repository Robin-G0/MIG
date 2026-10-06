#pragma once
#include <algorithm>
#include <cmath>
#include <mig/core/engine.hpp>
namespace mig::detail {
inline float dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}
inline bool finite(Vec2 p) {
    return std::isfinite(p.x) && std::isfinite(p.y);
}
inline bool usable(const Point& p) {
    return finite(p.position) && std::isfinite(p.confidence) && p.confidence >= 0.6f &&
           p.confidence <= 1 && p.position.x >= 0 && p.position.x <= 1 && p.position.y >= 0 &&
           p.position.y <= 1;
}
inline float distance_squared(Vec2 a, Vec2 b) {
    const auto difference = a - b;
    return dot(difference, difference);
}
inline float segment_distance_squared(Vec2 p, Vec2 a, Vec2 b) {
    const auto d = b - a;
    const float n = dot(d, d);
    return distance_squared(p, a + d * (n > 1e-8f ? std::clamp(dot(p - a, d) / n, 0.f, 1.f) : 0.f));
}
} // namespace mig::detail
