#include "../src/apps/action_output.hpp"
#include <iostream>
#include <stdexcept>
using namespace mig;
namespace {
void check(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}
Frame body(int time) {
    Frame frame;
    frame.sequence = frame.timestamp_ms = time;
    frame.aspect = 1;
    frame.points[11] = {{.65f, .45f}, 1};
    frame.points[12] = {{.35f, .45f}, 1};
    frame.points[15] = {{.7f, .65f}, 1};
    frame.points[16] = {{.3f, .65f}, 1};
    return frame;
}
void calibrate(Engine& engine) {
    for (int time = 50; time <= 1250; time += 50) {
        engine.update(body(time), time);
    }
    engine.restart();
    engine.set_test_mode(true);
}
Motion definition(ActionMode mode) {
    Motion input;
    input.id = input.name = input.action = "action";
    input.action_mode = mode;
    input.keyboard = ui::parse_binding("Ctrl+A");
    input.mirror = true;
    SpatialConstraint cell{"contact", 15, {4, 5}, ConstraintType::Interaction};
    cell.interaction = Interaction{HandSide::Left, Gesture::OK, 0};
    input.steps = {{"step", StepMode::Visited, {cell}}};
    return input;
}
Frame observe(const Engine& engine, int time, bool mirror = false, bool occupied = true,
              bool sign = true, float x = 4.5f, float y = 5.5f) {
    auto frame = body(time);
    const int side = mirror ? 1 : 0;
    frame.points[15 + side] = {
        engine.reference_grid().metric({mirror ? 9.f - x : x, occupied ? y : 8.5f}), 1};
    for (auto& finger : frame.fingers[side]) {
        finger = {1, 1};
    }
    frame.hand_contacts[side] = {sign ? .1f : .9f, 1};
    return frame;
}
} // namespace
int main() {
    try {
        for (auto mode : {ActionMode::SinglePress, ActionMode::Hold, ActionMode::Repeat}) {
            for (bool mirror : {false, true}) {
                Engine engine({{definition(mode)}});
                calibrate(engine);
                ui::ActionOutput output;
                std::vector<std::pair<int, bool>> sent;
                const auto send = [&](int key, bool up) {
                    sent.emplace_back(key, up);
                    return true;
                };
                const auto text = [](char16_t, bool) { return true; };
                for (int time = 1300; time <= 1800; time += 20) {
                    auto events = engine.update(observe(engine, time, mirror), time);
                    for (auto event : events) {
                        output.trigger(event.motion, engine.configuration().motions[0], time);
                    }
                    const std::array active{engine.action_active(0)};
                    check(active[0],
                          "Live output stays active while the firing hand holds its sign");
                    output.advance(engine.configuration().motions, active, time, send, text);
                }
                const auto presses =
                    std::count(sent.begin(), sent.end(), std::pair<int, bool>{65, false});
                check(presses == (mode == ActionMode::Repeat ? 3 : 1),
                      "Single/hold send once; Repeat uses its interval");
                check(mode != ActionMode::Hold ||
                          std::count(sent.begin(), sent.end(), std::pair<int, bool>{65, true}) == 0,
                      "Hold cannot expire after the normal 80ms pulse");
                engine.update(observe(engine, 1820, mirror, true, false), 1820);
                const std::array active{engine.action_active(0)};
                check(!active[0] && engine.progress(0, mirror).triggered,
                      "Losing the sign releases live output while Test retains its result");
                output.advance(engine.configuration().motions, active, 1820, send, text);
                check(output.empty(), "Loss of terminal conditions cancels holds and repeats");
                engine.update(observe(engine, 1840, mirror), 1840);
                check(!engine.action_active(0),
                      "A released latch cannot restart output without a new trigger");
            }
        }
        auto low_input = definition(ActionMode::Hold);
        SpatialConstraint low{"low", 15, {5, 5}, ConstraintType::Interaction, Priority::Low};
        low.tolerance_for = "contact";
        low.interaction = low_input.steps[0].constraints[0].interaction;
        low_input.steps[0].constraints.push_back(low);
        Engine low_engine({{low_input}});
        calibrate(low_engine);
        low_engine.update(observe(low_engine, 1300), 1300);
        low_engine.update(observe(low_engine, 1350, false, true, true, 5.5f), 1350);
        check(low_engine.action_active(0), "Low tolerance can sustain an accepted hold");
        low_engine.update(observe(low_engine, 1400, false, false), 1400);
        check(!low_engine.action_active(0), "Leaving both High and Low areas releases output");
        Engine stale({{definition(ActionMode::Hold)}});
        auto grace_input = definition(ActionMode::Hold);
        grace_input.steps[0].constraints[0].type = ConstraintType::Trigger;
        grace_input.steps[0].constraints[0].interaction.reset();
        grace_input.fingers = {{HandSide::Left, Finger::Index, FingerPose::Extended, 50, 150}};
        Engine grace({{grace_input}});
        calibrate(grace);
        grace.update(observe(grace, 1300), 1300);
        grace.update(observe(grace, 1350), 1350);
        auto finger_missing = observe(grace, 1400);
        finger_missing.fingers[0][1] = {};
        grace.update(finger_missing, 1400);
        check(grace.action_active(0),
              "Held output respects an already validated finger rule's configured grace");
        finger_missing.timestamp_ms = finger_missing.sequence = 1510;
        grace.update(finger_missing, 1510);
        check(!grace.action_active(0), "Finger grace expires and releases held output");
        calibrate(stale);
        stale.update(observe(stale, 1300), 1300);
        stale.update(observe(stale, 1350), 1700);
        check(!stale.action_active(0), "Stale tracking releases live action state");
        stale.restart();
        check(!stale.action_active(0), "Restart clears live action state");
        auto path = definition(ActionMode::Hold);
        path.steps = {{"path", StepMode::Ordered, {{"start", 15, {4, 5}}, {"end", 15, {4, 3}}}}};
        Engine final_pose({{path}});
        calibrate(final_pose);
        final_pose.update(observe(final_pose, 1300), 1300);
        check(final_pose.update(observe(final_pose, 1350, false, true, true, 4.5f, 3.5f), 1350)
                      .size() == 1,
              "Ordered path completes at its final pose");
        final_pose.update(observe(final_pose, 1400, false, true, true, 4.5f, 3.5f), 1400);
        check(final_pose.action_active(0),
              "Final ordered pose sustains output without retaining the start position");
        final_pose.update(observe(final_pose, 1450, false, false), 1450);
        check(!final_pose.action_active(0), "Leaving final ordered pose releases output");

        ui::ActionOutput shared;
        auto hold = definition(ActionMode::Hold), pulse = definition(ActionMode::SinglePress);
        pulse.id = "pulse";
        pulse.keyboard = ui::parse_binding("Ctrl+B");
        std::array motions{hold, pulse};
        std::array<bool, 2> active{true, true};
        std::vector<std::pair<int, bool>> keys;
        std::u16string typed;
        const auto send = [&](int key, bool up) {
            keys.emplace_back(key, up);
            return true;
        };
        const auto text = [&](char16_t unit, bool up) {
            if (!up) {
                typed += unit;
            }
            return true;
        };
        shared.trigger(0, hold, 100);
        shared.trigger(1, pulse, 100);
        shared.advance(motions, active, 100, send, text);
        shared.advance(motions, active, 180, send, text);
        check(std::count(keys.begin(), keys.end(), std::pair<int, bool>{17, false}) == 1 &&
                  std::count(keys.begin(), keys.end(), std::pair<int, bool>{17, true}) == 0,
              "Finishing a pulse must not release a modifier owned by Hold");
        active[0] = false;
        shared.advance(motions, active, 200, send, text);
        check(shared.empty() && keys.back() == std::pair<int, bool>{17, true},
              "Last owner releases shared modifier");
        hold.keyboard = ui::parse_binding("\"Hello\" _ Ctrl + C");
        motions[0] = hold;
        active[0] = true;
        keys.clear();
        shared.trigger(0, hold, 300);
        shared.advance(motions, active, 300, send, text);
        shared.advance(motions, active, 320, send, text);
        shared.advance(motions, active, 1000, send, text);
        check(typed == u"Hello" &&
                  std::count(keys.begin(), keys.end(), std::pair<int, bool>{67, true}) == 0,
              "Hold executes text prefix once and keeps only the final shortcut pressed");
        shared.cancel(send, text);
        check(shared.empty() && keys.back() == std::pair<int, bool>{17, true},
              "Stop cancels held final shortcut");
        auto fast = definition(ActionMode::Repeat);
        fast.repeat_interval_ms = 20;
        fast.keyboard = ui::parse_binding("A");
        std::array single{fast};
        std::array<bool, 1> live{true};
        keys.clear();
        shared.trigger(0, fast, 1100);
        for (int t = 1100; t <= 1150; t += 10) {
            shared.advance(single, live, t, send, text);
        }
        check(std::count(keys.begin(), keys.end(), std::pair<int, bool>{65, false}) == 3,
              "Short repeat intervals shorten their pulse and send distinct presses");
        shared.cancel(send, text);
        motions[0] = definition(ActionMode::SinglePress);
        motions[1] = motions[0];
        motions[1].id = "second";
        motions[0].keyboard = ui::parse_binding("\"abcdef\" _ Enter");
        motions[1].keyboard = ui::parse_binding("\"uvwxyz\" _ Enter");
        typed.clear();
        keys.clear();
        active.fill(true);
        shared.trigger(0, motions[0], 1200);
        shared.trigger(1, motions[1], 1200);
        for (int t = 1200; t <= 1500; t += 20) {
            shared.advance(motions, active, t, send, text);
        }
        check(typed == u"abcdefuvwxyz" &&
                  std::count(keys.begin(), keys.end(), std::pair<int, bool>{13, false}) == 2,
              "Simultaneous accepted text sequences remain serialized, never interleaved");
        shared.cancel(send, text);
        hold = definition(ActionMode::Hold);
        motions[0] = hold;
        shared.trigger(0, hold, 1600);
        shared.advance(motions, active, 1600, send, text);
        bool fail_release = true;
        const auto retry_release = [&](int key, bool up) {
            return !(fail_release && up && key == 65);
        };
        shared.cancel(retry_release, text);
        check(!shared.empty(), "Failed release retains ownership for retry");
        fail_release = false;
        shared.cancel(retry_release, text);
        check(shared.empty(), "Failed releases can be retried without leaving keys held");
        auto invalid = definition(ActionMode::Hold);
        invalid.keyboard = ui::parse_binding("\"text only\"");
        bool rejected = false;
        try {
            validate(Configuration{{invalid}});
        } catch (const std::exception&) {
            rejected = true;
        }
        check(rejected, "Hold text-only expressions must report the required final shortcut");
        invalid = definition(ActionMode::Repeat);
        invalid.repeat_interval_ms = 0;
        rejected = false;
        try {
            validate(Configuration{{invalid}});
        } catch (const std::exception&) {
            rejected = true;
        }
        check(rejected, "Repeat interval zero cannot create an unbounded output loop");
        std::cout << "Action modes, live/Mirror/terminal state, hold/sequence/repeat and shared "
                     "modifier cleanup passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
