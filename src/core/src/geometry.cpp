#include "geometry.hpp"
namespace mig {
using namespace detail;
float distance(Vec2 a, Vec2 b) {
    return std::hypot(a.x - b.x, a.y - b.y);
}
Vec2 Grid::local(Vec2 p) const {
    const auto d = p - center;
    return {dot(d, axis) / scale + 4.5f, dot(d, {-axis.y, axis.x}) / scale + 3.5f};
}
Vec2 Grid::metric(Vec2 p) const {
    return center + axis * ((p.x - 4.5f) * scale) + Vec2{-axis.y, axis.x} * ((p.y - 3.5f) * scale);
}
} // namespace mig
