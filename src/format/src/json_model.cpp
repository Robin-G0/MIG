#include "json_model.hpp"
#include <mig/core/engine.hpp>
#include <stdexcept>

namespace mig::format::detail {
using Json = nlohmann::json;
namespace {
void fields(const Json& object, std::initializer_list<std::string_view> allowed) {
    if (!object.is_object()) {
        throw std::runtime_error("Expected a JSON object");
    }
    for (auto it = object.begin(); it != object.end(); ++it) {
        if (std::find(allowed.begin(), allowed.end(), it.key()) == allowed.end()) {
            throw std::runtime_error("Unknown configuration field: " + it.key());
        }
    }
}
int integer(const Json& value, int low, int high) {
    if (!value.is_number_integer() ||
        (value.is_number_unsigned() && value.get<std::uint64_t>() > std::uint64_t(high))) {
        throw std::runtime_error("Expected a bounded integer");
    }
    const auto result = value.get<std::int64_t>();
    if (result < low || result > high) {
        throw std::runtime_error("Integer out of range");
    }
    return int(result);
}
template <class Enum>
Enum choice(const Json& value, std::initializer_list<std::string_view> names) {
    const auto text = value.get<std::string>();
    int index = 0;
    for (auto name : names) {
        if (name == text) {
            return static_cast<Enum>(index);
        }
        ++index;
    }
    throw std::runtime_error("Unknown option: " + text);
}
template <class Enum> std::string label(Enum value, std::initializer_list<std::string_view> names) {
    return std::string(*(names.begin() + int(value)));
}
void array(const Json& value, std::size_t limit) {
    if (!value.is_array() || value.size() > limit) {
        throw std::runtime_error("Expected a bounded array");
    }
}
std::vector<FingerConstraint> read_fingers(const Json& list) {
    array(list, 10);
    std::vector<FingerConstraint> result;
    for (const auto& item : list) {
        fields(item, {"hand", "finger", "pose", "stable_ms", "grace_ms"});
        FingerConstraint finger;
        finger.hand = choice<HandSide>(item.at("hand"), {"left", "right"});
        finger.finger =
            choice<Finger>(item.at("finger"), {"thumb", "index", "middle", "ring", "pinky"});
        finger.pose = choice<FingerPose>(item.at("pose"), {"extended", "closed"});
        finger.stable_ms = integer(item.value("stable_ms", Json(100)), 50, 500);
        finger.grace_ms = integer(item.value("grace_ms", Json(150)), 0, 300);
        result.push_back(finger);
    }
    return result;
}
Json write_fingers(const std::vector<FingerConstraint>& fingers) {
    auto result = Json::array();
    for (const auto& finger : fingers) {
        result.push_back(
            {{"hand", label(finger.hand, {"left", "right"})},
             {"finger", label(finger.finger, {"thumb", "index", "middle", "ring", "pinky"})},
             {"pose", label(finger.pose, {"extended", "closed"})},
             {"stable_ms", finger.stable_ms},
             {"grace_ms", finger.grace_ms}});
    }
    return result;
}
std::vector<SpatialConstraint> read_constraints(const Json& list) {
    array(list, 1024);
    std::vector<SpatialConstraint> result;
    for (const auto& item : list) {
        fields(item, {"id", "landmark", "cell", "type", "priority", "tolerance_for", "fingers",
                      "order", "interaction"});
        SpatialConstraint constraint;
        constraint.id = item.at("id").get<std::string>();
        constraint.landmark = landmark_index(item.at("landmark").get<std::string>());
        const auto& cell = item.at("cell");
        array(cell, 4);
        if (cell.size() != 2 && cell.size() != 4) {
            throw std::runtime_error("cell must be [x,y] or [x,y,width,height]");
        }
        constraint.cell.x = integer(cell[0], Grid::min_cell, Grid::max_cell - 1);
        constraint.cell.y = integer(cell[1], Grid::min_cell, Grid::max_cell - 1);
        if (cell.size() == 4) {
            constraint.cell.width = integer(cell[2], 1, Grid::cell_count);
            constraint.cell.height = integer(cell[3], 1, Grid::cell_count);
        }
        constraint.type = choice<ConstraintType>(
            item.at("type"), {"required", "forbidden", "trigger", "interaction"});
        constraint.priority = choice<Priority>(item.at("priority"), {"high", "low"});
        constraint.order = integer(item.value("order", Json(0)), 0, 1024);
        constraint.tolerance_for = item.value("tolerance_for", "");
        constraint.fingers = read_fingers(item.value("fingers", Json::array()));
        if (item.contains("interaction")) {
            const auto& settings = item.at("interaction");
            fields(settings, {"hand", "gesture", "hold_ms"});
            constraint.interaction = Interaction{
                choice<HandSide>(settings.at("hand"), {"left", "right"}),
                choice<Gesture>(settings.at("gesture"),
                                {"none", "thumb", "v", "ok", "open_palm", "fist"}),
                integer(settings.value("hold_ms", Json(0)), 0, maximum_interaction_hold_ms)};
        }
        result.push_back(std::move(constraint));
    }
    return result;
}
Json write_constraints(const std::vector<SpatialConstraint>& constraints) {
    auto result = Json::array();
    for (const auto& constraint : constraints) {
        result.push_back(
            {{"id", constraint.id},
             {"landmark", landmark_name(constraint.landmark)},
             {"cell",
              {constraint.cell.x, constraint.cell.y, constraint.cell.width,
               constraint.cell.height}},
             {"type", label(constraint.type, {"required", "forbidden", "trigger", "interaction"})},
             {"priority", label(constraint.priority, {"high", "low"})},
             {"tolerance_for", constraint.tolerance_for},
             {"fingers", write_fingers(constraint.fingers)},
             {"order", constraint.order}});
        if (constraint.interaction) {
            const auto& settings = *constraint.interaction;
            result.back()["interaction"] = {
                {"hand", label(settings.hand, {"left", "right"})},
                {"gesture",
                 label(settings.gesture, {"none", "thumb", "v", "ok", "open_palm", "fist"})},
                {"hold_ms", settings.hold_ms}};
        }
    }
    return result;
}
ControlBinding read_binding(const Json& item) {
    fields(item, {"gesture", "hand"});
    return {choice<Gesture>(item.at("gesture"), {"none", "thumb", "v", "ok", "open_palm", "fist"}),
            choice<HandSide>(item.at("hand"), {"left", "right"})};
}
Json write_binding(const ControlBinding& binding) {
    return {{"gesture", label(binding.gesture, {"none", "thumb", "v", "ok", "open_palm", "fist"})},
            {"hand", label(binding.hand, {"left", "right"})}};
}
} // namespace
Configuration read_v2(const Json& document) {
    fields(document, {"schema_version", "tracking", "controls", "inputs"});
    Configuration config;
    if (document.contains("tracking")) {
        fields(document["tracking"], {"hands"});
        config.track_hands = document["tracking"].value("hands", false);
    }
    if (document.contains("controls")) {
        const auto& controls = document["controls"];
        fields(controls, {"restart", "recalibrate", "record_toggle", "validation_ms",
                          "post_gesture_delay_ms"});
        if (controls.contains("restart")) {
            config.controls.restart = read_binding(controls["restart"]);
        }
        if (controls.contains("recalibrate")) {
            config.controls.recalibrate = read_binding(controls["recalibrate"]);
        }
        if (controls.contains("record_toggle")) {
            config.controls.record_toggle = read_binding(controls["record_toggle"]);
        }
        config.controls.validation_ms =
            integer(controls.value("validation_ms", Json(250)), 100, 1000);
        config.controls.post_gesture_delay_ms =
            integer(controls.value("post_gesture_delay_ms", Json(1000)), 1000, 1000);
    }
    array(document.at("inputs"), 64);
    for (const auto& item : document["inputs"]) {
        fields(item, {"id", "name", "action", "keyboard", "mirror", "space", "max_duration_ms",
                      "cooldown_ms", "constraints", "steps", "fingers", "recordings", "action_mode",
                      "repeat_interval_ms"});
        Motion input;
        input.id = item.at("id").get<std::string>();
        input.name = item.at("name").get<std::string>();
        input.action = item.at("action").get<std::string>();
        if (item.contains("keyboard")) {
            array(item["keyboard"], maximum_keyboard_actions);
            for (const auto& action : item["keyboard"]) {
                fields(action, {"keys", "text"});
                if (action.contains("keys") == action.contains("text")) {
                    throw std::runtime_error("Keyboard action requires keys or text, exclusively");
                }
                KeyboardAction parsed;
                if (action.contains("text")) {
                    parsed.type = KeyboardActionType::Text;
                    parsed.text = action.at("text").get<std::string>();
                } else {
                    array(action.at("keys"), 255);
                    for (const auto& key : action.at("keys")) {
                        parsed.keys.push_back(integer(key, 1, 255));
                    }
                }
                input.keyboard.push_back(std::move(parsed));
            }
        }
        input.mirror = item.value("mirror", false);
        input.action_mode = choice<ActionMode>(item.value("action_mode", Json("single_press")),
                                               {"single_press", "hold", "repeat"});
        input.repeat_interval_ms = integer(item.value("repeat_interval_ms", Json(200)), 20, 60000);
        input.space = choice<CoordinateSpace>(item.value("space", Json("calibrated")),
                                              {"body", "calibrated"});
        input.max_duration_ms = integer(item.value("max_duration_ms", Json(0)), 0, 10000);
        input.cooldown_ms = integer(item.value("cooldown_ms", Json(0)), 0, 10000);
        input.constraints = read_constraints(item.value("constraints", Json::array()));
        input.fingers = read_fingers(item.value("fingers", Json::array()));
        const auto steps = item.value("steps", Json::array());
        array(steps, 64);
        for (const auto& item_step : steps) {
            fields(item_step, {"id", "mode", "hold_ms", "constraints", "fingers"});
            Step step;
            step.id = item_step.at("id").get<std::string>();
            step.mode = choice<StepMode>(item_step.value("mode", Json("visited")),
                                         {"visited", "simultaneous", "ordered"});
            step.hold_ms = integer(item_step.value("hold_ms", Json(0)), 0, 1000);
            step.constraints = read_constraints(item_step.at("constraints"));
            step.fingers = read_fingers(item_step.value("fingers", Json::array()));
            input.steps.push_back(std::move(step));
        }
        const auto traces = item.value("recordings", Json::array());
        array(traces, 34);
        for (const auto& trace : traces) {
            fields(trace, {"landmark", "points"});
            RecordedTrace recording;
            recording.landmark = landmark_index(trace.at("landmark").get<std::string>());
            array(trace.at("points"), 2048);
            for (const auto& point : trace["points"]) {
                array(point, 2);
                if (point.size() != 2 || !point[0].is_number() || !point[1].is_number()) {
                    throw std::runtime_error("Invalid recorded point");
                }
                recording.points.push_back({point[0].get<float>(), point[1].get<float>()});
            }
            input.recordings.push_back(std::move(recording));
        }
        config.motions.push_back(std::move(input));
    }
    validate(config);
    return config;
}
Json write_v2(const Configuration& config) {
    Json document{{"schema_version", 2},
                  {"tracking", {{"hands", config.track_hands}}},
                  {"controls",
                   {{"restart", write_binding(config.controls.restart)},
                    {"recalibrate", write_binding(config.controls.recalibrate)},
                    {"record_toggle", write_binding(config.controls.record_toggle)},
                    {"validation_ms", config.controls.validation_ms},
                    {"post_gesture_delay_ms", 1000}}},
                  {"inputs", Json::array()}};
    for (const auto& input : config.motions) {
        Json item{{"id", input.id},
                  {"name", input.name.empty() ? input.id : input.name},
                  {"action", input.action.empty() ? input.id : input.action},
                  {"keyboard", Json::array()},
                  {"mirror", input.mirror},
                  {"space", label(input.space, {"body", "calibrated"})},
                  {"max_duration_ms", input.max_duration_ms},
                  {"cooldown_ms", input.cooldown_ms},
                  {"action_mode", label(input.action_mode, {"single_press", "hold", "repeat"})},
                  {"repeat_interval_ms", input.repeat_interval_ms},
                  {"constraints", write_constraints(input.constraints)},
                  {"fingers", write_fingers(input.fingers)},
                  {"steps", Json::array()},
                  {"recordings", Json::array()}};
        for (const auto& action : input.keyboard) {
            item["keyboard"].push_back(action.type == KeyboardActionType::Text
                                           ? Json{{"text", action.text}}
                                           : Json{{"keys", action.keys}});
        }
        for (const auto& step : input.steps) {
            item["steps"].push_back(
                {{"id", step.id},
                 {"mode", label(step.mode, {"visited", "simultaneous", "ordered"})},
                 {"hold_ms", step.hold_ms},
                 {"constraints", write_constraints(step.constraints)},
                 {"fingers", write_fingers(step.fingers)}});
        }
        for (const auto& trace : input.recordings) {
            item["recordings"].push_back(
                {{"landmark", landmark_name(trace.landmark)}, {"points", trace.points}});
        }
        document["inputs"].push_back(std::move(item));
    }
    return document;
}
} // namespace mig::format::detail
