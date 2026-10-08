#pragma once
#include "keybindings.hpp"
#include <mig/core/engine.hpp>

namespace mig::ui {
inline std::string review_text(std::string_view value) {
    std::string result;
    constexpr char hex[] = "0123456789abcdef";
    for (const auto character : quote_text(value)) {
        const auto byte = static_cast<unsigned char>(character);
        if (byte < 32 || byte == 127) {
            result += "\\u00";
            result += hex[byte >> 4];
            result += hex[byte & 15];
        } else {
            result += character;
        }
    }
    return result;
}
struct ReviewedAction {
    std::string category{"Keyboard input"};
    std::string description;
    bool system_interaction{};
};
inline std::vector<ReviewedAction> review_actions(const Motion& input) {
    std::vector<ReviewedAction> result;
    for (const auto& action : input.keyboard) {
        result.push_back({"Keyboard input",
                          action.type == KeyboardActionType::Text
                              ? "Type: " + review_text(action.text)
                              : "Chord: " + shortcut_label(action.keys),
                          system_keyboard_action(action)});
    }
    return result;
}
inline std::string configuration_review(const Configuration& config) {
    std::string result =
        "MIG maps movements to keyboard inputs. Shortcuts and typed text can interact with "
        "the operating system. Review every mapping before importing.\n"
        "System interaction labels are a heuristic, not a safety guarantee.\n"
        "Import does not enable keyboard output; enable it separately after review.\n\n";
    for (const auto& input : config.motions) {
        result += "Movement: " + review_text(input.name.empty() ? input.id : input.name) + " (ID " +
                  review_text(input.id) + ")\n";
        result += "Mode: " + std::string(action_mode_names.at(std::size_t(input.action_mode)));
        if (input.action_mode == ActionMode::Repeat) {
            result += " every " + std::to_string(input.repeat_interval_ms) + " ms";
        } else if (input.action_mode == ActionMode::Hold) {
            result += " (prefix once, final chord held while active)";
        }
        result += "\n";
        const auto actions = review_actions(input);
        if (actions.empty()) {
            result += "Logical action only: " + review_text(input.action) + "\n";
        }
        for (std::size_t index = 0; index < actions.size(); ++index) {
            const auto& action = actions[index];
            result +=
                std::to_string(index + 1) + ". " + action.category + " / " +
                (action.system_interaction ? "System interaction" : "Standard keyboard input") +
                ": " + action.description + "\n";
        }
        result += "\n";
    }
    return result;
}
} // namespace mig::ui
