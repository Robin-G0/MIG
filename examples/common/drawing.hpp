#pragma once
#include "source.hpp"
namespace demo {
template <class Polygon>
void draw_regions(const mig::Engine& engine, float aspect, Polygon polygon) {
    const auto& grid = engine.grid();
    if (!grid.valid) {
        return;
    }
    for (int row = 1; row <= 5; ++row) {
        std::array<mig::Vec2, 4> points;
        unsigned index = 0;
        for (const auto local : {mig::Vec2{-9, float(row)}, mig::Vec2{18, float(row)},
                                 mig::Vec2{18, float(row + 1)}, mig::Vec2{-9, float(row + 1)}}) {
            const auto metric = grid.metric(local);
            points[index++] = {1 - metric.x / aspect, metric.y};
        }
        polygon(points, row == 1);
    }
}
template <class Line, class Dot> void draw(const Source& source, Line line, Dot dot) {
    for (const auto& point : mig::body_coordinates(source.body)) {
        if (point.confidence >= .6f) {
            dot(point.position.x, point.position.y);
        }
    }
    for (std::size_t index = 0; index < source.hands.count; ++index) {
        const auto& hand = source.hands.hands[index];
        for (int base : {1, 5, 9, 13, 17}) {
            auto previous = hand.points[0];
            for (int joint = base; joint < base + 4; ++joint) {
                const auto point = hand.points[joint];
                line(previous.x, previous.y, point.x, point.y);
                dot(point.x, point.y);
                previous = point;
            }
        }
    }
}
} // namespace demo
