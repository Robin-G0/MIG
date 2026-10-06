#include "geometry.hpp"
namespace mig {
using namespace detail;
bool Engine::update_grid(const Frame& f, std::int64_t previous_time) noexcept {
    const auto metric = [&](Vec2 p) { return Vec2{p.x * f.aspect, p.y}; };
    // MediaPipe anatomical right is normally image-left in a front-facing image.
    const auto left = metric(f.points[12].position), right = metric(f.points[11].position);
    const float width = distance(left, right);
    if (width < 0.08f || right.x <= left.x) {
        lose_tracking();
        return false;
    }
    const auto center = (left + right) * 0.5f, axis = (right - left) * (1.f / width);
    if (!grid_.calibrated) {
        if (stable_since_ < 0 || distance(center, anchor_) > width * 0.1f ||
            (sample_count_ && std::abs(width - calibration_reference_width_) >
                                  calibration_reference_width_ * 0.1f)) {
            stable_since_ = f.timestamp_ms;
            sample_count_ = 0;
            width_cursor_ = 0;
            calibration_reference_width_ = width;
            anchor_ = center;
        }
        widths_[width_cursor_] = width;
        width_cursor_ = (width_cursor_ + 1) % widths_.size();
        sample_count_ = std::min(sample_count_ + 1, widths_.size());
        if (f.timestamp_ms - stable_since_ < 1000 || sample_count_ < 10) {
            return false;
        }
        std::nth_element(widths_.begin(), widths_.begin() + sample_count_ / 2,
                         widths_.begin() + sample_count_);
        grid_.baseline_width = widths_[sample_count_ / 2];
        grid_.calibrated = true;
        grid_.center = center;
        grid_.axis = axis;
        reference_grid_ = grid_;
    }
    const bool continuity =
        grid_.valid && previous_time >= 0 && f.timestamp_ms - previous_time <= 180;
    const float alpha =
        continuity ? 1.f - std::exp(-float(f.timestamp_ms - previous_time) / 35.f) : 1.f;
    grid_.center = grid_.center + (center - grid_.center) * alpha;
    auto filtered = grid_.axis + (axis - grid_.axis) * alpha;
    grid_.axis = filtered * (1.f / std::max(distance(filtered, {}), 1e-6f));
    const float scale = width * Grid::cell_shoulder_ratio;
    grid_.scale = grid_.scale + (scale - grid_.scale) * alpha;
    grid_.approach =
        std::clamp(1.f - grid_.baseline_width / width, -1.f, 0.8f); // Apparent-scale proxy only.
    grid_.valid = true;
    reference_grid_.scale = grid_.scale;
    reference_grid_.valid = true;

    return true;
}
} // namespace mig
