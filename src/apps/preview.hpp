#pragma once
#include <mig/core/engine.hpp>

namespace mig::ui {
// Presentation only: inference and saved paths retain unmirrored coordinates.
struct PreviewTransform {
    float left, top, width, height, aspect;
    [[nodiscard]] Vec2 screen(Vec2 metric) const noexcept {
        return {left + (1.f - metric.x / aspect) * (width - 1.f), top + metric.y * (height - 1.f)};
    }
    [[nodiscard]] Vec2 metric(Vec2 screen) const noexcept {
        return {(1.f - (screen.x - left) / (width - 1.f)) * aspect,
                (screen.y - top) / (height - 1.f)};
    }
};
// One projection for grid rendering and hit testing. The live view uses the
// inference frame's aspect, shoulder scale, centre and axis; the offline view
// keeps the full editable grid available before calibration.
struct GridView {
    PreviewTransform preview{};
    Grid grid{};
    float left{}, top{}, side{};
    bool live{};
    [[nodiscard]] Vec2 screen(Vec2 point) const noexcept {
        return live ? preview.screen(grid.metric(point))
                    : Vec2{left + (Grid::max_cell - point.x) * side / Grid::cell_count,
                           top + (point.y - Grid::min_cell) * side / Grid::cell_count};
    }
    [[nodiscard]] Vec2 local(Vec2 point) const noexcept {
        return live ? grid.local(preview.metric(point))
                    : Vec2{Grid::max_cell - (point.x - left) * Grid::cell_count / side,
                           Grid::min_cell + (point.y - top) * Grid::cell_count / side};
    }
};
} // namespace mig::ui
