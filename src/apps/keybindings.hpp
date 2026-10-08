#pragma once
#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <deque>
#include <limits>
#include <mig/core/input.hpp>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace mig::ui {
inline constexpr std::pair<std::string_view, int> key_names[]{
    {"Ctrl", 17}, {"Shift", 16},  {"Alt", 18},      {"Win", 91},    {"Space", 32},  {"Enter", 13},
    {"Tab", 9},   {"Esc", 27},    {"Backspace", 8}, {"Delete", 46}, {"Insert", 45}, {"Home", 36},
    {"End", 35},  {"PageUp", 33}, {"PageDown", 34}, {"Left", 37},   {"Up", 38},     {"Right", 39},
    {"Down", 40}, {"Plus", 187},  {"Minus", 189}};
inline bool modifier(int key) {
    return key == 16 || key == 17 || key == 18 || key == 91 || key == 92 ||
           (key >= 160 && key <= 165);
}
inline std::string lowercase(std::string_view text) {
    std::string value(text);
    for (auto& c : value) {
        if (c >= 'A' && c <= 'Z') {
            c += 'a' - 'A';
        }
    }
    return value;
}
inline std::vector<int> parse_shortcut(std::string_view text) {
    std::vector<int> keys;
    if (text.empty() || lowercase(text) == "none" || text == "0") {
        return keys;
    }
    while (!text.empty()) {
        const auto separator = text.find('+');
        auto token = text.substr(0, separator);
        while (!token.empty() && token.front() == ' ') {
            token.remove_prefix(1);
        }
        while (!token.empty() && token.back() == ' ') {
            token.remove_suffix(1);
        }
        const auto name = lowercase(token);
        int key = 0;
        for (const auto& entry : key_names) {
            if (lowercase(entry.first) == name) {
                key = entry.second;
            }
        }
        if (name == "control") {
            key = 17;
        }
        if (name == "escape") {
            key = 27;
        }
        if (token.size() == 1 &&
            ((name[0] >= 'a' && name[0] <= 'z') || (name[0] >= '0' && name[0] <= '9'))) {
            key = name[0] >= 'a' ? name[0] - 'a' + 'A' : name[0];
        }
        if (name.size() > 1 && name[0] == 'f') {
            int number{};
            const auto parsed = std::from_chars(name.data() + 1, name.data() + name.size(), number);
            if (parsed.ec == std::errc{} && parsed.ptr == name.data() + name.size() &&
                number >= 1 && number <= 24) {
                key = 111 + number;
            }
        }
        if (name.starts_with("vk:")) {
            const auto parsed = std::from_chars(name.data() + 3, name.data() + name.size(), key);
            if (parsed.ec != std::errc{} || parsed.ptr != name.data() + name.size()) {
                key = 0;
            }
        }
        if (!supported_keyboard_key(key) || keys.size() >= 255 ||
            std::find(keys.begin(), keys.end(), key) != keys.end()) {
            throw std::runtime_error("Unknown/duplicate shortcut key; use _ to repeat, e.g. A _ A");
        }
        keys.push_back(key);
        if (separator == std::string_view::npos) {
            break;
        }
        text.remove_prefix(separator + 1);
        if (text.empty()) {
            throw std::runtime_error("Shortcut cannot end with +");
        }
    }
    std::stable_partition(keys.begin(), keys.end(), modifier);
    return keys;
}
inline std::string shortcut_label(std::span<const int> keys) {
    std::string result;
    for (int key : keys) {
        std::string name;
        for (const auto& entry : key_names) {
            if (entry.second == key) {
                name = entry.first;
            }
        }
        if (name.empty()) {
            if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) {
                name = char(key);
            } else if (key >= 112 && key <= 135) {
                name = "F" + std::to_string(key - 111);
            } else {
                name = "VK:" + std::to_string(key);
            }
        }
        if (!result.empty()) {
            result += '+';
        }
        result += name;
    }
    return result.empty() ? "None" : result;
}
// One deadline per virtual key preserves shared modifiers across overlapping
// chords. The sender is injectable so tests never generate desktop keystrokes.
class ChordOutput {
public:
    ChordOutput() {
        pressed_.reserve(255);
    }
    template <class Send>
    bool pulse(std::span<const int> keys, std::int64_t now, Send send, bool hold = false,
               int pulse_ms = 80) {
        if (keys.size() > 255 ||
            std::any_of(keys.begin(), keys.end(), [](int key) { return key < 1 || key > 255; })) {
            return false;
        }
        std::array<int, 255> ordered{};
        std::size_t size = 0;
        for (bool mods : {true, false}) {
            for (int key : keys) {
                if (modifier(key) == mods) {
                    ordered[size++] = key;
                }
            }
        }
        std::array<int, 255> added{};
        std::size_t count = 0;
        for (int key : std::span(ordered.data(), size)) {
            if (held_[key]) {
                continue;
            }
            if (!send(key, false)) {
                while (count > 0) {
                    const auto rollback = added[--count];
                    if (send(rollback, true)) {
                        until_[rollback] = 0;
                        held_[rollback] = false;
                        std::erase(pressed_, rollback);
                    }
                }
                return false;
            }
            added[count++] = key;
            pressed_.push_back(key);
            held_[key] = true;
            until_[key] = hold ? std::numeric_limits<std::int64_t>::max() : now + pulse_ms;
        }
        for (int key : keys) {
            until_[key] = std::max(until_[key], hold ? std::numeric_limits<std::int64_t>::max()
                                                     : now + pulse_ms);
        }
        return true;
    }
    template <class Send> void expire(std::int64_t now, Send send) {
        for (std::size_t i = pressed_.size(); i > 0; --i) {
            const int key = pressed_[i - 1];
            if (now >= until_[key] && send(key, true)) {
                until_[key] = 0;
                held_[key] = false;
                pressed_.erase(pressed_.begin() + i - 1);
            }
        }
    }
    template <class Send> void release(Send send) {
        for (int key : pressed_) {
            until_[key] = 0;
        }
        expire(0, send);
    }
    bool empty() const noexcept {
        return pressed_.empty();
    }

private:
    std::array<std::int64_t, 256> until_{};
    std::array<bool, 256> held_{};
    std::vector<int> pressed_;
};
inline std::string_view trimmed(std::string_view value) {
    while (!value.empty() && (value.front() == ' ' || value.front() == '\n' ||
                              value.front() == '\r' || value.front() == '\t')) {
        value.remove_prefix(1);
    }
    while (!value.empty() && (value.back() == ' ' || value.back() == '\n' || value.back() == '\r' ||
                              value.back() == '\t')) {
        value.remove_suffix(1);
    }
    return value;
}
inline std::vector<KeyboardAction> parse_binding(std::string_view value) {
    value = trimmed(value);
    if (value.empty() || lowercase(value) == "none" || value == "0") {
        return {};
    }
    std::vector<KeyboardAction> result;
    while (!value.empty()) {
        KeyboardAction action;
        if (value.front() == '"') {
            action.type = KeyboardActionType::Text;
            value.remove_prefix(1);
            bool closed = false;
            while (!value.empty()) {
                char c = value.front();
                value.remove_prefix(1);
                if (c == '"') {
                    closed = true;
                    break;
                }
                if (c == '\\') {
                    if (value.empty()) {
                        throw std::runtime_error("Incomplete text escape");
                    }
                    c = value.front();
                    value.remove_prefix(1);
                    if (c == 'n') {
                        c = '\n';
                    } else if (c == 't') {
                        c = '\t';
                    } else if (c == 'r') {
                        c = '\r';
                    } else if (c != '"' && c != '\\') {
                        throw std::runtime_error("Use \\\" or \\\\ inside quoted text");
                    }
                }
                action.text.push_back(c);
            }
            if (!closed) {
                throw std::runtime_error("Close the quoted text with \"");
            }
            value = trimmed(value);
            if (!value.empty() && value.front() != '_') {
                throw std::runtime_error("Separate text and shortcuts with _");
            }
        } else {
            const auto separator = value.find('_');
            auto chord = trimmed(value.substr(0, separator));
            if (chord.find('"') != std::string_view::npos) {
                throw std::runtime_error("Text must be quoted and separated with _");
            }
            action.keys = parse_shortcut(chord);
            if (action.keys.empty()) {
                throw std::runtime_error("An empty action is not a shortcut");
            }
            value =
                separator == std::string_view::npos ? std::string_view{} : value.substr(separator);
        }
        result.push_back(std::move(action));
        if (result.size() > maximum_keyboard_actions) {
            throw std::runtime_error("Maximum 256 keyboard actions");
        }
        if (!value.empty()) {
            value.remove_prefix(1);
            value = trimmed(value);
            if (value.empty()) {
                throw std::runtime_error("Binding cannot end with _");
            }
        }
    }
    validate_keyboard(result);
    return result;
}
inline std::string quote_text(std::string_view value) {
    std::string result = "\"";
    for (auto c : value) {
        if (c == '"' || c == '\\') {
            result += '\\';
            result += c;
        } else if (c == '\n') {
            result += "\\n";
        } else if (c == '\t') {
            result += "\\t";
        } else if (c == '\r') {
            result += "\\r";
        } else {
            result += c;
        }
    }
    return result + '"';
}
inline std::string binding_label(std::span<const KeyboardAction> actions) {
    std::string result;
    for (const auto& action : actions) {
        if (!result.empty()) {
            result += " _ ";
        }
        result += action.type == KeyboardActionType::Text ? quote_text(action.text)
                                                          : shortcut_label(action.keys);
    }
    return result.empty() ? "None" : result;
}
inline std::vector<std::string> binding_tokens(std::span<const KeyboardAction> actions) {
    std::vector<std::string> result;
    for (const auto& action : actions) {
        if (!result.empty()) {
            result.push_back("_");
        }
        if (action.type == KeyboardActionType::Text) {
            result.push_back(quote_text(action.text));
        } else {
            for (std::size_t i = 0; i < action.keys.size(); ++i) {
                if (i > 0) {
                    result.push_back("+");
                }
                const std::array single{action.keys[i]};
                result.push_back(shortcut_label(single));
            }
        }
    }
    return result;
}
// Runs are serialized: all modifiers are released before text or the next chord.
// Tick advances bounded work; no sleeps or desktop key injection in unit tests.
class KeyboardOutput {
public:
    bool enqueue(const std::vector<KeyboardAction>& actions, bool hold_last = false,
                 int pulse_ms = 80) {
        if (actions.empty()) {
            return true;
        }
        if (runs_.size() >= 16) {
            return false;
        }
        validate_keyboard(actions);
        if (hold_last && actions.back().type != KeyboardActionType::Chord) {
            throw std::runtime_error("Hold requires a final shortcut");
        }
        runs_.push_back({actions, hold_last, pulse_ms});
        return true;
    }
    template <class Send, class TextSend>
    bool advance(std::int64_t now, Send send, TextSend send_text, bool allow_text = true) {
        chord_.expire(now, send);
        if (!chord_.empty()) {
            return true;
        }
        if (unicode_held_) {
            if (!send_text(*unicode_held_, true)) {
                return false;
            }
            unicode_held_.reset();
        }
        if (runs_.empty() || now < next_) {
            return true;
        }
        auto& run = runs_.front();
        const auto& action = run.actions[action_];
        if (action.type == KeyboardActionType::Chord) {
            const auto duration = run.pulse_ms;
            const bool hold_final = run.hold_last && action_ + 1 == run.actions.size();
            if (!chord_.pulse(action.keys, now, send, hold_final, duration)) {
                cancel(send, send_text);
                return false;
            }
            holding_ = hold_final;
            finish_action();
            next_ = now + duration + std::min(20, duration);
            return true;
        }
        if (!allow_text) {
            return true;
        }
        if (text_.empty() && text_cursor_ == 0) {
            text_ = keyboard_text(action.text);
        }
        std::size_t count = 0;
        while (text_cursor_ < text_.size() && count++ < 32) {
            const auto unit = text_[text_cursor_++];
            if (!send_text(unit, false)) {
                cancel(send, send_text);
                return false;
            }
            unicode_held_ = unit;
            if (!send_text(unit, true)) {
                cancel(send, send_text);
                return false;
            }
            unicode_held_.reset();
        }
        if (text_cursor_ == text_.size()) {
            finish_action();
            next_ = now + 20;
        }
        return true;
    }
    template <class Send, class TextSend> void cancel(Send send, TextSend send_text) {
        runs_.clear();
        action_ = text_cursor_ = 0;
        text_.clear();
        next_ = 0;
        holding_ = false;
        chord_.release(send);
        if (unicode_held_ && send_text(*unicode_held_, true)) {
            unicode_held_.reset();
        }
    }
    bool empty() const noexcept {
        return runs_.empty() && chord_.empty() && !unicode_held_;
    }
    bool executing() const noexcept {
        return !runs_.empty() || unicode_held_ || (!chord_.empty() && !holding_);
    }

private:
    void finish_action() {
        text_.clear();
        text_cursor_ = 0;
        if (++action_ == runs_.front().actions.size()) {
            runs_.pop_front();
            action_ = 0;
        }
    }
    ChordOutput chord_;
    struct Run {
        std::vector<KeyboardAction> actions;
        bool hold_last{};
        int pulse_ms{80};
    };
    std::deque<Run> runs_;
    std::size_t action_{}, text_cursor_{};
    std::u16string text_;
    std::optional<char16_t> unicode_held_;
    std::int64_t next_{};
    bool holding_{};
};
} // namespace mig::ui
