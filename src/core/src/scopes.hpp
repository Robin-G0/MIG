#pragma once
#include "geometry.hpp"
#include <limits>

namespace mig::detail {
template <class Candidate, class Hit, class FingerMatch>
bool evaluate_scope(Candidate& candidate, const std::array<bool, 34>& visible, const Hit& hit,
                    const FingerMatch& finger_match, std::size_t begin, std::size_t end,
                    StepMode mode) noexcept {
    auto& progress = candidate.progress;
    bool complete = true;
    std::array<bool, 34> blocked{};
    std::array<float, 34> cursors{};
    bool after_trigger = false;
    for (std::size_t slot = begin; slot < end; ++slot) {
        const auto index = candidate.evaluation_order[slot];
        if (candidate.group_root[index] != index) {
            continue;
        }
        const auto& constraint = *candidate.constraints[index];
        const bool fingers_ok = finger_match(constraint.fingers);
        float reached = hit(constraint, mode != StepMode::Simultaneous);
        if (!visible[constraint.landmark] && constraint.priority == Priority::High) {
            progress.constraints[index] = ConstraintStatus::LandmarkLost;
            progress.failed = true;
            complete = false;
            continue;
        }
        if (constraint.type == ConstraintType::Forbidden) {
            if (reached >= 0) {
                progress.constraints[index] = ConstraintStatus::Forbidden;
                progress.failed = true;
            }
            continue;
        }
        if (constraint.priority == Priority::Low) {
            continue;
        }
        if (mode == StepMode::Ordered && after_trigger) {
            continue; // Follow-through is metadata after the firing prefix.
        }
        bool matched = false;
        bool validated = false;
        std::size_t matched_index = index, tolerance_index = index;
        float earliest = std::numeric_limits<float>::max();
        for (const auto member : candidate.groups[index]) {
            const auto& region = *candidate.constraints[member];
            if (mode != StepMode::Simultaneous) {
                validated |= progress.constraints[member] == ConstraintStatus::Validated;
            }
            float contact = hit(region, mode != StepMode::Simultaneous);
            auto tolerance = member;
            const bool region_fingers = finger_match(region.fingers);
            if (contact < 0 && region_fingers) {
                for (const auto alternative : candidate.alternatives[member]) {
                    const auto& low = *candidate.constraints[alternative];
                    const auto low_contact = hit(low, mode != StepMode::Simultaneous);
                    if (low_contact >= 0 && finger_match(low.fingers) &&
                        (contact < 0 || low_contact < contact)) {
                        contact = low_contact;
                        tolerance = alternative;
                    }
                }
            }
            if (contact >= 0 && !region_fingers) {
                progress.constraints[member] = ConstraintStatus::FingerInvalid;
            }
            if (contact >= 0 && region_fingers && contact < earliest) {
                matched = true;
                earliest = contact;
                matched_index = member;
                tolerance_index = tolerance;
            }
        }
        if (matched) {
            reached = earliest;
        }
        if (mode == StepMode::Ordered &&
            (blocked[constraint.landmark] ||
             (reached >= 0 && reached < cursors[constraint.landmark]))) {
            matched = false;
        }
        if (mode == StepMode::Ordered && is_trigger(constraint.type)) {
            after_trigger = true;
            if (reached >= 0 && reached < cursors[constraint.landmark]) {
                complete = false;
            }
        }
        if (matched && !is_trigger(constraint.type)) {
            progress.constraints[matched_index] = ConstraintStatus::Validated;
            progress.constraints[tolerance_index] = ConstraintStatus::Validated;
            validated = true;
            cursors[constraint.landmark] = std::max(cursors[constraint.landmark], reached);
        } else if (mode == StepMode::Simultaneous && !is_trigger(constraint.type)) {
            progress.constraints[index] = reached >= 0 && !fingers_ok
                                              ? ConstraintStatus::FingerInvalid
                                              : ConstraintStatus::Missing;
        } else if (reached >= 0 && !fingers_ok) {
            progress.constraints[index] = ConstraintStatus::FingerInvalid;
        }
        if (!is_trigger(constraint.type) && !validated) {
            complete = false;
            blocked[constraint.landmark] = true;
        }
    }
    return complete;
}
} // namespace mig::detail
