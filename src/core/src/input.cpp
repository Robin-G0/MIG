#include <algorithm>
#include <mig/core/input.hpp>
#include <stdexcept>

namespace mig {
bool supported_keyboard_key(int key) noexcept {
    // Defined keyboard VKs only: exclude mouse, gamepad, reserved values and
    // VK_PACKET (Unicode text has its own validated representation).
    return key == 3 || key == 8 || key == 9 || key == 12 || key == 13 || (key >= 16 && key <= 57) ||
           (key >= 65 && key <= 93) || (key >= 95 && key <= 135) || (key >= 144 && key <= 150) ||
           (key >= 160 && key <= 183) || (key >= 186 && key <= 192) || (key >= 219 && key <= 223) ||
           key == 225 || key == 226 || key == 229 || (key >= 246 && key <= 254);
}
bool system_keyboard_action(const KeyboardAction& action) noexcept {
    if (action.type != KeyboardActionType::Chord) {
        return false;
    }
    const auto contains = [&](int key) {
        return std::find(action.keys.begin(), action.keys.end(), key) != action.keys.end();
    };
    const bool alt = contains(18) || contains(164) || contains(165);
    const bool ctrl = contains(17) || contains(162) || contains(163);
    // Heuristic, not a sandbox. Include common window/session controls and
    // desktop launch keys; text following these remains visible in the review.
    return contains(91) || contains(92) || contains(95) ||
           (alt && (contains(115) || contains(9) || contains(27) || contains(32))) ||
           (ctrl && alt &&
            (contains(46) || contains(8) ||
             std::any_of(action.keys.begin(), action.keys.end(),
                         [](int key) { return key >= 112 && key <= 123; }))) ||
           (ctrl && (contains(16) || contains(160) || contains(161)) && contains(27)) ||
           std::any_of(action.keys.begin(), action.keys.end(),
                       [](int key) { return key >= 180 && key <= 183; });
}
std::u16string keyboard_text(std::string_view text) {
    std::u16string result;
    for (std::size_t i = 0; i < text.size();) {
        const auto first = static_cast<unsigned char>(text[i++]);
        std::uint32_t code = first;
        int following = 0;
        std::uint32_t minimum = 0;
        if (first >= 0xc2 && first <= 0xdf) {
            code = first & 31;
            following = 1;
            minimum = 0x80;
        } else if (first >= 0xe0 && first <= 0xef) {
            code = first & 15;
            following = 2;
            minimum = 0x800;
        } else if (first >= 0xf0 && first <= 0xf4) {
            code = first & 7;
            following = 3;
            minimum = 0x10000;
        } else if (first >= 0x80) {
            throw std::runtime_error("Keyboard text must be valid UTF-8");
        }
        for (int n = 0; n < following; ++n) {
            if (i == text.size() || (static_cast<unsigned char>(text[i]) & 0xc0) != 0x80) {
                throw std::runtime_error("Keyboard text must be valid UTF-8");
            }
            code = (code << 6) | (static_cast<unsigned char>(text[i++]) & 63);
        }
        if (code == 0 || code < minimum || code > 0x10ffff || (code >= 0xd800 && code <= 0xdfff)) {
            throw std::runtime_error("Invalid Unicode character in keyboard text");
        }
        if (code <= 0xffff) {
            result.push_back(char16_t(code));
        } else {
            code -= 0x10000;
            result.push_back(char16_t(0xd800 + (code >> 10)));
            result.push_back(char16_t(0xdc00 + (code & 1023)));
        }
    }
    return result;
}
void validate_keyboard(const std::vector<KeyboardAction>& sequence) {
    if (sequence.size() > maximum_keyboard_actions) {
        throw std::runtime_error("Maximum 256 keyboard actions");
    }
    std::size_t bytes = 0;
    std::size_t keys = 0;
    for (const auto& action : sequence) {
        if (action.type == KeyboardActionType::Text) {
            if (!action.keys.empty()) {
                throw std::runtime_error("Text actions cannot contain shortcut keys");
            }
            bytes += action.text.size();
            if (bytes > maximum_keyboard_text_bytes) {
                throw std::runtime_error("Maximum 16384 bytes of keyboard text per input");
            }
            keyboard_text(action.text);
        } else if (action.type == KeyboardActionType::Chord) {
            if (!action.text.empty() || action.keys.empty() || action.keys.size() > 255) {
                throw std::runtime_error("A shortcut needs keys only");
            }
            std::array<bool, 256> used{};
            keys += action.keys.size();
            if (keys > maximum_keyboard_keys) {
                throw std::runtime_error("Maximum 1024 serialized shortcut keys per input");
            }
            for (int key : action.keys) {
                if (!supported_keyboard_key(key) || used[key]) {
                    throw std::runtime_error(
                        "Duplicate/invalid shortcut key; use _ to repeat a key in sequence");
                }
                used[key] = true;
            }
        } else {
            throw std::runtime_error("Unknown keyboard action type");
        }
    }
}
int landmark_index(std::string_view name) {
    for (const auto& landmark : landmarks) {
        if (landmark.name == name) {
            return landmark.index;
        }
    }
    throw std::runtime_error("Unknown landmark: " + std::string(name));
}
std::string_view landmark_name(int index) {
    for (const auto& landmark : landmarks) {
        if (landmark.index == index) {
            return landmark.name;
        }
    }
    throw std::runtime_error("Unknown landmark index");
}
int mirrored_landmark(int index) noexcept {
    for (const auto& landmark : landmarks) {
        if (landmark.index == index) {
            return landmark.mirror_index;
        }
    }
    return index;
}
} // namespace mig
