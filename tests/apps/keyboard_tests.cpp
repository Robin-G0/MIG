#include "../../src/apps/import_review.hpp"
#include "../../src/apps/keybindings.hpp"
#include <iostream>
#include <stdexcept>
using namespace mig::ui;
void check(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}
int main() {
    try {
        const auto keys = parse_shortcut("K + Ctrl + Shift");
        check(keys == std::vector<int>{17, 16, 75} && shortcut_label(keys) == "Ctrl+Shift+K",
              "Shortcut parser puts modifiers first and formats readable combinations");
        check(parse_shortcut("None").empty() &&
                  parse_shortcut("F24+Space") == std::vector<int>{135, 32},
              "None and named keys");
        for (auto text : {"Ctrl+Ctrl", "A+", "F25", "VK:256"}) {
            bool rejected = false;
            try {
                parse_shortcut(text);
            } catch (const std::exception&) {
                rejected = true;
            }
            check(rejected, "Malformed shortcuts rejected");
        }
        ChordOutput output;
        std::vector<std::pair<int, bool>> sent;
        const auto send = [&](int key, bool release) {
            sent.emplace_back(key, release);
            return true;
        };
        check(output.pulse(keys, 100, send), "Chord press");
        check(sent == std::vector<std::pair<int, bool>>{{17, false}, {16, false}, {75, false}},
              "Modifiers pressed before ordinary key");
        output.expire(179, send);
        check(sent.size() == 3, "Chord held for 80 ms");
        output.expire(180, send);
        check(
            output.empty() &&
                sent ==
                    std::vector<std::pair<int, bool>>{
                        {17, false}, {16, false}, {75, false}, {75, true}, {16, true}, {17, true}},
            "Chord released in reverse order");
        sent.clear();
        output.pulse(parse_shortcut("Ctrl+A"), 200, send);
        output.pulse(parse_shortcut("Ctrl+B"), 240, send);
        output.expire(280, send);
        check(sent.back() == std::pair<int, bool>{65, true} &&
                  std::count(sent.begin(), sent.end(), std::pair<int, bool>{17, true}) == 0,
              "Overlapping chords retain their shared modifier");
        output.release(send);
        check(output.empty() && sent.back() == std::pair<int, bool>{17, true},
              "Stop/disable releases all remaining keys");
        sent.clear();
        const auto fail = [&](int key, bool release) {
            sent.emplace_back(key, release);
            return release || key != 75;
        };
        check(!output.pulse(keys, 400, fail) && output.empty() &&
                  sent[sent.size() - 2] == std::pair<int, bool>{16, true} &&
                  sent.back() == std::pair<int, bool>{17, true},
              "Failed chord rolls back keys that were already pressed");
        std::cout << "Shortcut parsing, chord output, overlap and cleanup passed\n";
        const auto binding = parse_binding("\"Hello world\" _ Enter _ Ctrl + C _ C _ C");
        check(binding.size() == 5 && binding[0].text == "Hello world" &&
                  binding[1].keys == std::vector<int>{13} &&
                  parse_binding(binding_label(binding)) == binding,
              "Text, chronology, simultaneous shortcuts and repeated keys roundtrip");
        check(parse_binding("A+B+C+D+E+F+G+H+I")[0].keys.size() == 9,
              "The eight-key chord limit is removed");
        const auto literal = parse_binding("\"a_b+c\\\"\\\\\" _ Enter");
        check(literal[0].text == "a_b+c\"\\", "Quoted operators and escapes remain literal text");
        for (auto source :
             {"A _", "_ A", "\"unclosed", "\"text\" + A", "A _ _ B", "None _ A", "A + A"}) {
            bool bad = false;
            try {
                parse_binding(source);
            } catch (const std::exception&) {
                bad = true;
            }
            check(bad, "Malformed sequences cannot be accepted");
        }
        KeyboardOutput sequence;
        sent.clear();
        std::u16string typed;
        const auto text_send = [&](char16_t unit, bool release) {
            if (!release) {
                typed += unit;
            }
            return true;
        };
        check(sequence.enqueue(binding), "Sequence queued");
        sequence.advance(1000, send, text_send);
        check(typed == u"Hello world" && sent.empty(), "Quoted text precedes Enter");
        sequence.advance(1020, send, text_send);
        check(sent == std::vector<std::pair<int, bool>>{{13, false}}, "Enter follows text");
        sequence.advance(1100, send, text_send);
        check(sent.back() == std::pair<int, bool>{13, true}, "Enter released before shortcut");
        sequence.advance(1120, send, text_send);
        check(sent[sent.size() - 2] == std::pair<int, bool>{17, false},
              "Next chord presses Ctrl first");
        for (int t = 1140; t <= 1500; t += 20) {
            sequence.advance(t, send, text_send);
        }
        check(sequence.empty() &&
                  std::count(sent.begin(), sent.end(), std::pair<int, bool>{67, false}) == 3,
              "Repeated C sends distinct down/up pairs");
        typed.clear();
        sent.clear();
        sequence.enqueue(parse_binding("Ctrl + A _ \"é😀\" _ Enter"));
        for (int t = 1600; t < 1900; t += 20) {
            sequence.advance(t, send, text_send);
        }
        check(typed == u"é😀" && sent[3] == std::pair<int, bool>{17, true},
              "Modifiers release before Unicode including surrogate pairs");
        sequence.enqueue(parse_binding("A _ B _ C"));
        sequence.advance(2000, send, text_send);
        const auto before_cancel = sent.size();
        sequence.cancel(send, text_send);
        sequence.advance(2200, send, text_send);
        check(sequence.empty() && sent.size() == before_cancel + 1 &&
                  sent.back() == std::pair<int, bool>{65, true},
              "Cancellation releases pressed keys and drops remaining actions");
        check(mig::keyboard_text("é😀") == u"é😀", "Strict UTF-8 conversion");
        bool invalid_utf8 = false;
        try {
            mig::keyboard_text(std::string("\xc0\xaf", 2));
        } catch (const std::exception&) {
            invalid_utf8 = true;
        }
        check(invalid_utf8, "Overlong UTF-8 rejected");
        for (auto source : {"VK:1", "VK:7", "VK:58", "VK:136", "VK:195", "VK:231", "VK:255"}) {
            bool bad = false;
            try {
                parse_binding(source);
            } catch (const std::exception&) {
                bad = true;
            }
            check(bad, "Mouse, gamepad, reserved and Unicode-packet keys rejected");
        }
        check(mig::system_keyboard_action(parse_binding("Win+R")[0]) &&
                  mig::system_keyboard_action(parse_binding("Alt+F4")[0]) &&
                  mig::system_keyboard_action(parse_binding("VK:163+VK:165+Delete")[0]) &&
                  !mig::system_keyboard_action(parse_binding("Ctrl+C")[0]) &&
                  !mig::system_keyboard_action(parse_binding("Space")[0]),
              "System shortcut heuristic handles sided modifiers and ordinary game keys");
        mig::Motion reviewed;
        reviewed.id = reviewed.name = "Right wrist";
        reviewed.keyboard = parse_binding("Win+R _ \"hello\\nworld\" _ Enter");
        reviewed.action_mode = mig::ActionMode::Repeat;
        reviewed.repeat_interval_ms = 400;
        const auto summary = configuration_review(mig::Configuration{{reviewed}});
        check(review_text(std::string("a\0b", 3)) == "\"a\\u0000b\"" &&
                  review_text(std::string("\x1b", 1)) == "\"\\u001b\"",
              "Control characters in names/text cannot truncate or disguise the review");
        check(summary.find("System interaction: Chord: Win+R") != std::string::npos &&
                  summary.find(
                      "2. Keyboard input / Standard keyboard input: Type: \"hello\\nworld\"") !=
                      std::string::npos &&
                  summary.find("Repeat every 400 ms") != std::string::npos,
              "Review preserves full event order, escaped text, classification and repeat mode");
        const auto reject_sequence = [](const auto& actions) {
            try {
                mig::validate_keyboard(actions);
            } catch (const std::exception&) {
                return true;
            }
            return false;
        };
        std::vector<mig::KeyboardAction> bounded(mig::maximum_keyboard_actions,
                                                 parse_binding("Space")[0]);
        mig::validate_keyboard(bounded);
        bounded.push_back(bounded.front());
        check(reject_sequence(bounded), "Action count bound applies to serialized actions");
        auto text_action = parse_binding("\"x\"")[0];
        text_action.text.assign(mig::maximum_keyboard_text_bytes, 'x');
        mig::validate_keyboard({text_action});
        text_action.text += 'x';
        check(reject_sequence(std::vector{text_action}),
              "Text byte bound rejects without truncating");
        std::vector<mig::KeyboardAction> chords(256, parse_binding("A+B+C+D")[0]);
        mig::validate_keyboard(chords);
        chords[0].keys.push_back('E');
        check(reject_sequence(chords),
              "Total serialized key count bounded independently of action count");
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
