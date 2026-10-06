#pragma once
#include "authoring.hpp"
#include <limits>

namespace mig::ui {
inline bool contains(const Cell& cell, Vec2 point) {
    return point.x >= cell.x && point.x < cell.x + cell.width && point.y >= cell.y &&
           point.y < cell.y + cell.height;
}
struct RegionSelection {
    Vec2 origin{}, end{};
    bool dragging{};
    std::vector<std::string> ids, previous;
    void clear() {
        ids.clear();
        previous.clear();
        dragging = false;
    }
    void begin(Vec2 point, bool additive) {
        origin = end = point;
        previous = additive ? ids : std::vector<std::string>{};
        dragging = true;
    }
    void update(const std::vector<SpatialConstraint>& regions, Vec2 point, int landmark) {
        end = point;
        ids = previous;
        const float left = std::min(origin.x, end.x), right = std::max(origin.x, end.x);
        const float top = std::min(origin.y, end.y), bottom = std::max(origin.y, end.y);
        for (const auto& region : regions) {
            const auto& cell = region.cell;
            const bool hit = origin == end ? contains(cell, point)
                                           : cell.x < right && cell.x + cell.width > left &&
                                                 cell.y < bottom && cell.y + cell.height > top;
            if (region.landmark == landmark && hit &&
                std::find(ids.begin(), ids.end(), region.id) == ids.end()) {
                ids.push_back(region.id);
            }
        }
    }
    bool includes(const std::string& id) const {
        return std::find(ids.begin(), ids.end(), id) != ids.end();
    }
};
inline void erase_regions(std::vector<SpatialConstraint>& regions,
                          const std::vector<std::string>& ids) {
    const auto removed = [&](const std::string& id) {
        return std::find(ids.begin(), ids.end(), id) != ids.end();
    };
    std::erase_if(regions, [&](const auto& region) {
        return removed(region.id) ||
               (!region.tolerance_for.empty() && removed(region.tolerance_for));
    });
}
inline void clear_fingers(Motion& motion) {
    motion.fingers.clear();
    const auto clear = [](auto& regions) {
        for (auto& region : regions) {
            region.fingers.clear();
        }
    };
    clear(motion.constraints);
    for (auto& step : motion.steps) {
        step.fingers.clear();
        clear(step.constraints);
    }
}
inline std::string contour_target(const std::vector<SpatialConstraint>& regions, Vec2 point,
                                  int landmark) {
    for (const auto& region : regions) {
        if (region.landmark == landmark && contains(region.cell, point) &&
            region.priority == Priority::High) {
            return region.id;
        }
    }
    for (const auto& region : regions) {
        if (region.landmark == landmark && contains(region.cell, point) &&
            !region.tolerance_for.empty()) {
            return region.tolerance_for;
        }
    }
    return {};
}
inline std::string tolerance_target(const std::vector<SpatialConstraint>& regions,
                                    const SpatialConstraint& brush, Vec2 point) {
    float nearest = std::numeric_limits<float>::max();
    std::string target;
    for (const auto& region : regions) {
        if (region.priority != Priority::High || region.landmark != brush.landmark ||
            region.type != brush.type) {
            continue;
        }
        const auto& cell = region.cell;
        const float dx = point.x - std::clamp(point.x, float(cell.x), float(cell.x + cell.width));
        const float dy = point.y - std::clamp(point.y, float(cell.y), float(cell.y + cell.height));
        const float separation = dx * dx + dy * dy;
        if (separation < nearest) {
            target = region.id;
            nearest = separation;
        }
    }
    return target;
}
} // namespace mig::ui
