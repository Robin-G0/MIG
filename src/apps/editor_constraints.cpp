#include "app.hpp"
namespace mig::app {
void App::update_constraint() {
    auto& list = current_constraints();
    if (selected_constraint < 0 || std::size_t(selected_constraint) >= list.size()) {
        throw std::runtime_error("Select a constraint");
    }
    auto& item = list[selected_constraint];
    item.landmark = landmarks[selection(edit_window, Members)].index;
    item.type = ConstraintType(selection(edit_window, ConstraintTypeId));
    if (item.type == ConstraintType::Interaction) {
        item.interaction = Interaction{
            HandSide(selection(edit_window, InteractionHand)),
            Gesture(selection(edit_window, InteractionGesture) + 1),
            integer_value(edit_window, InteractionHold, 0, maximum_interaction_hold_ms)};
    } else {
        item.interaction.reset();
    }
    item.priority = Priority(selection(edit_window, PriorityId));
    if (scope > 0 && draft.steps[scope - 1].mode == StepMode::Ordered &&
        item.priority == Priority::High && item.type != ConstraintType::Forbidden) {
        const int chosen_order = selection(edit_window, BrushOrder);
        const bool numbered = std::any_of(list.begin(), list.end(), [&](const auto& other) {
            return other.landmark == item.landmark && other.order > 0;
        });
        if (chosen_order > 0 || numbered) {
            ui::number_lane(list, item.landmark);
            if (chosen_order > 0) {
                item.order = chosen_order;
            }
        } else {
            item.order = 0;
        }
    } else {
        item.order = 0;
    }
    item.cell = {integer_value(edit_window, CellX, Grid::min_cell, Grid::max_cell - 1),
                 integer_value(edit_window, CellY, Grid::min_cell, Grid::max_cell - 1),
                 integer_value(edit_window, CellWidth, 1, Grid::cell_count),
                 integer_value(edit_window, CellHeight, 1, Grid::cell_count)};
    if (item.priority == Priority::High) {
        item.tolerance_for.clear();
    }
    for (auto& low : list) {
        if (low.tolerance_for == item.id) {
            low.landmark = item.landmark;
            low.type = item.type;
            low.interaction = item.interaction;
        }
    }
}
void App::delete_constraint() {
    auto& list = current_constraints();
    if (selected_constraint < 0 || std::size_t(selected_constraint) >= list.size()) {
        return;
    }
    const auto removed = list[selected_constraint].id;
    std::erase_if(list, [&](const auto& item) {
        return item.id == removed || item.tolerance_for == removed;
    });
    selected_constraint = -1;
}
void App::reorder_constraint(int id) {
    auto& list = current_constraints();
    if (selected_constraint >= 0 && std::size_t(selected_constraint) < list.size() &&
        list[selected_constraint].order > 0) {
        const auto selected_region = list[selected_constraint];
        int adjacent = id == ConstraintUp ? 0 : 1025;
        for (const auto& item : list) {
            if (item.landmark != selected_region.landmark || item.order == 0) {
                continue;
            }
            if (id == ConstraintUp && item.order < selected_region.order) {
                adjacent = std::max(adjacent, item.order);
            } else if (id == ConstraintDown && item.order > selected_region.order) {
                adjacent = std::min(adjacent, item.order);
            }
        }
        if (adjacent > 0 && adjacent <= 1024) {
            for (auto& item : list) {
                if (item.landmark == selected_region.landmark) {
                    if (item.order == selected_region.order) {
                        item.order = adjacent;
                    } else if (item.order == adjacent) {
                        item.order = selected_region.order;
                    }
                }
            }
        }
        return;
    }
    const int next = selected_constraint + (id == ConstraintUp ? -1 : 1);
    if (selected_constraint >= 0 && next >= 0 && std::size_t(next) < list.size()) {
        std::swap(list[selected_constraint], list[next]);
        selected_constraint = next;
    }
}
void App::update_interaction() {
    auto& list = current_constraints();
    if (selected_constraint < 0 || std::size_t(selected_constraint) >= list.size() ||
        list[selected_constraint].type != ConstraintType::Interaction) {
        throw std::runtime_error(
            "Select a purple Interaction cell first, or draw with these settings");
    }
    auto& region = list[selected_constraint];
    region.interaction =
        Interaction{HandSide(selection(edit_window, InteractionHand)),
                    Gesture(selection(edit_window, InteractionGesture) + 1),
                    integer_value(edit_window, InteractionHold, 0, maximum_interaction_hold_ms)};
    const auto target = region.priority == Priority::High ? region.id : region.tolerance_for;
    for (auto& other : list) {
        if (other.id == target || other.tolerance_for == target) {
            other.interaction = region.interaction;
        }
    }
}
void App::add_finger() {
    auto& fingers = current_fingers();
    FingerConstraint finger{HandSide(selection(edit_window, FingerHand)),
                            Finger(selection(edit_window, FingerId)),
                            FingerPose(selection(edit_window, FingerPoseId)),
                            integer_value(edit_window, FingerStable, 50, 500),
                            integer_value(edit_window, FingerGrace, 0, 300)};
    std::erase_if(fingers, [&](const auto& item) {
        return item.hand == finger.hand && item.finger == finger.finger;
    });
    fingers.push_back(finger);
}
void App::delete_finger() {
    auto& fingers = current_fingers();
    if (selected_finger >= 0 && std::size_t(selected_finger) < fingers.size()) {
        fingers.erase(fingers.begin() + selected_finger);
    }
}
void App::delete_layer() {
    const auto landmark = landmarks[selection(edit_window, Members)].index;
    const auto remove = [&](auto& constraints) {
        std::erase_if(constraints, [&](const auto& item) { return item.landmark == landmark; });
    };
    remove(draft.constraints);
    for (auto& step : draft.steps) {
        remove(step.constraints);
    }
    selected_constraint = -1;
}
void App::edit_authoring(int id) {
    const auto before = draft;
    try {
        switch (id) {
        case Mirror:
            draft.mirror = !draft.mirror;
            break;
        case AddStep:
            if (draft.steps.size() == 64) {
                throw std::runtime_error("Maximum 64 steps");
            }
            draft.steps.push_back({"step_" + std::to_string(++serial)});
            scope = int(draft.steps.size());
            break;
        case DeleteStep:
            if (scope > 0) {
                draft.steps.erase(draft.steps.begin() + scope - 1);
                --scope;
            }
            break;
        case StepUp:
        case StepDown: {
            const int next = scope + (id == StepUp ? -1 : 1);
            if (scope > 0 && next > 0 && next <= int(draft.steps.size())) {
                std::swap(draft.steps[scope - 1], draft.steps[next - 1]);
                scope = next;
            }
            break;
        }
        case Trigger:
            if (scope > 0) {
                draft.steps[scope - 1].hold_ms = integer_value(edit_window, HoldTime, 0, 1000);
            }
            break;
        case ConvertRecording: {
            std::lock_guard lock(state_mutex);
            draft = reviewed_recording.convert(draft);
            scope = 1;
            break;
        }
        case UpdateConstraint:
            update_constraint();
            break;
        case DeleteConstraint:
            delete_constraint();
            break;
        case ConstraintUp:
        case ConstraintDown:
            reorder_constraint(id);
            break;
        case SetInteraction:
            update_interaction();
            break;
        case AddFinger:
            add_finger();
            break;
        case DeleteFinger:
            delete_finger();
            break;
        case DeleteLayer:
            delete_layer();
            break;
        case Clear:
            if (pro_mode) {
                current_constraints().clear();
            } else {
                draft.constraints.clear();
                draft.steps = {{"step_1", StepMode::Ordered}};
                draft.recordings.clear();
                scope = 1;
                selected_constraint = -1;
            }
            break;
        default:
            return;
        }
    } catch (...) {
        draft = before;
        throw;
    }
    if (draft != before) {
        remember_edit(before);
    }
    refresh_editor();
}
} // namespace mig::app
