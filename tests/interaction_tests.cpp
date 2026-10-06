#include "../src/apps/keybindings.hpp"
#include <iostream>
#include <mig/core/engine.hpp>
#include <mig/core/session.hpp>
#include <stdexcept>
using namespace mig;
namespace {
void check(bool value, const char* message) {
    if (!value) {
        throw std::runtime_error(message);
    }
}
Frame body(int t) {
    Frame frame;
    frame.timestamp_ms = frame.sequence = t;
    frame.aspect = 1;
    frame.points[11] = {{.65f, .45f}, 1};
    frame.points[12] = {{.35f, .45f}, 1};
    frame.points[15] = {{.7f, .65f}, 1};
    frame.points[16] = {{.3f, .65f}, 1};
    return frame;
}
Motion definition(int hold = 200) {
    Motion input;
    input.id = "interact";
    input.mirror = true;
    SpatialConstraint cell{"purple", 15, {4, 5}, ConstraintType::Interaction};
    cell.interaction = Interaction{HandSide::Left, Gesture::OK, hold};
    input.steps = {Step{"sign", StepMode::Ordered, {cell}}};
    return input;
}
void calibrate(Engine& engine) {
    for (int t = 50; t <= 1250; t += 50) {
        engine.update(body(t), t);
    }
    engine.restart();
}
Frame observation(const Engine& engine, int t, bool mirrored = false, bool inside = true,
                  bool sign = true) {
    auto frame = body(t);
    const auto metric =
        engine.reference_grid().metric({mirrored ? 9.f - 4.5f : 4.5f, inside ? 5.5f : 7.5f});
    const int side = mirrored ? 1 : 0;
    frame.points[15 + side] = {metric, 1};
    for (auto& finger : frame.fingers[side]) {
        finger = {1, 1};
    }
    frame.hand_contacts[side] = {sign ? .1f : .8f, 1};
    return frame;
}
} // namespace
int main() {
    try {
        for (bool mirror : {false, true}) {
            Engine engine({{definition()}});
            calibrate(engine);
            for (int t = 1300; t < 1500; t += 50) {
                check(engine.update(observation(engine, t, mirror), t).empty(),
                      "Hold must elapse inside the cell");
            }
            check(engine.update(observation(engine, 1500, mirror), 1500).size() == 1,
                  "Interaction fires on the matching anatomical hand and mirrored side");
            check(engine.progress(0, mirror).constraints[0] == ConstraintStatus::Triggered,
                  "Interaction progress highlights the firing cell");
            check(engine.update(observation(engine, 1550, mirror), 1550).empty(),
                  "Held sign cannot repeat");
        }
        Engine reset({{definition()}});
        for (bool mirror : {false, true}) {
            Engine long_hold({{definition(12000)}});
            calibrate(long_hold);
            for (int t = 1300; t < 13300; t += 200) {
                check(long_hold.update(observation(long_hold, t, mirror), t).empty(),
                      "A twelve-second hold cannot fire early");
                const auto& progress = long_hold.progress(0, mirror);
                check(!progress.failed && progress.constraints[0] == ConstraintStatus::Holding &&
                          progress.interaction_elapsed_ms[0] == t - 1300,
                      "Long hold exposes elapsed time without invalidating the input");
            }
            check(long_hold.update(observation(long_hold, 13300, mirror), 13300).size() == 1,
                  "Long hold fires on both anatomical hands");
        }
        Engine diagnostics({{definition()}});
        auto scoped = definition();
        scoped.fingers = {{HandSide::Left, Finger::Index, FingerPose::Extended, 100, 0}};
        Engine retry({{scoped}});
        calibrate(retry);
        retry.update(observation(retry, 1300), 1300);
        retry.update(observation(retry, 1400), 1400);
        auto curled = observation(retry, 1450);
        curled.fingers[0][1].extension = 0;
        retry.update(curled, 1450);
        check(!retry.progress(0).failed && !retry.progress(0).fingers_valid,
              "Pure Interaction finger mismatch restarts its hold without locking the input");
        retry.update(observation(retry, 1500), 1500);
        check(retry.update(observation(retry, 1600), 1600).empty(), "Recover stable finger rule");
        check(retry.update(observation(retry, 1750), 1750).empty(),
              "Restart the full hold after mismatch");
        check(retry.update(observation(retry, 1800), 1800).size() == 1,
              "Recovered fingers can trigger without leaving the purple region");
        calibrate(diagnostics);
        auto no_hand = observation(diagnostics, 1300);
        no_hand.fingers = {};
        diagnostics.update(no_hand, 1300);
        check(diagnostics.progress(0).constraints[0] == ConstraintStatus::HandMissing,
              "Missing hand observations are distinct from an incorrect sign");
        diagnostics.update(observation(diagnostics, 1400, false, true, false), 1400);
        check(diagnostics.progress(0).constraints[0] == ConstraintStatus::SignMismatch,
              "Known fingers with an incorrect sign show sign mismatch");
        calibrate(reset);
        check(reset.update(observation(reset, 1300), 1300).empty(), "Start hold");
        check(reset.update(observation(reset, 1400, false, true, false), 1400).empty(),
              "Separated fingertips reject OK");
        check(reset.update(observation(reset, 1450), 1450).empty(), "Mismatch resets dwell");
        check(reset.update(observation(reset, 1600), 1600).empty(),
              "Old partial hold cannot be reused");
        check(reset.update(observation(reset, 1650), 1650).size() == 1,
              "New continuous hold succeeds");

        Engine leave({{definition()}});
        calibrate(leave);
        leave.update(observation(leave, 1300), 1300);
        check(leave.update(observation(leave, 1400, false, false), 1400).empty(),
              "Leaving cell never triggers by sweep");
        check(leave.update(observation(leave, 1450), 1450).empty(), "Reentry resets dwell");
        check(leave.update(observation(leave, 1600), 1600).empty(), "New dwell still incomplete");
        check(leave.update(observation(leave, 1650), 1650).size() == 1,
              "Reentry continuous hold fires");

        Engine unknown({{definition()}});
        calibrate(unknown);
        unknown.update(observation(unknown, 1300), 1300);
        auto missing = observation(unknown, 1400);
        missing.hand_contacts = {};
        check(unknown.update(missing, 1400).empty(), "Unknown contact is not OK");
        check(unknown.update(observation(unknown, 1450), 1450).empty(),
              "Unknown contact resets hold");
        check(unknown.update(observation(unknown, 1600), 1600).empty(),
              "Hold cannot span occlusion");
        check(unknown.update(observation(unknown, 1650), 1650).size() == 1,
              "Recovery after occlusion");

        Engine stale({{definition()}});
        calibrate(stale);
        stale.update(observation(stale, 1300), 1300);
        check(stale.update(observation(stale, 1400), 1700).empty(), "Stale observation rejected");
        check(stale.update(observation(stale, 1750), 1750).empty(),
              "Stale gap cannot retain dwell");

        Engine immediate({{definition(0)}});
        calibrate(immediate);
        check(immediate.update(observation(immediate, 1300), 1300).size() == 1,
              "Zero hold fires immediately");
        Engine wrong_hand({{definition(0)}});
        calibrate(wrong_hand);
        auto wrong = observation(wrong_hand, 1300);
        wrong.fingers[1] = wrong.fingers[0];
        wrong.hand_contacts[1] = wrong.hand_contacts[0];
        wrong.fingers[0] = {};
        wrong.hand_contacts[0] = {};
        check(wrong_hand.update(wrong, 1300).empty(),
              "Opposite-hand sign cannot satisfy the original cell");

        auto tolerance_input = definition(100);
        auto low = tolerance_input.steps[0].constraints[0];
        low.id = "low";
        low.priority = Priority::Low;
        low.tolerance_for = "purple";
        low.cell = {4, 7};
        tolerance_input.steps[0].constraints.push_back(low);
        Engine tolerance({{tolerance_input}});
        calibrate(tolerance);
        check(tolerance.update(observation(tolerance, 1300, false, false), 1300).empty(),
              "Linked Interaction tolerance starts the same hold");
        check(tolerance.update(observation(tolerance, 1400, false, false), 1400).size() == 1,
              "Interaction tolerance accepts contact after hold");

        Engine crossing({{definition(0)}});
        calibrate(crossing);
        crossing.update(observation(crossing, 1300, false, false), 1300);
        auto above = observation(crossing, 1400, false, false);
        above.points[15] = {crossing.reference_grid().metric({4.5f, 4.5f}), 1};
        check(crossing.update(above, 1400).empty(),
              "Sweeping past purple cell is not an interaction");

        auto prefix = definition(100);
        prefix.steps[0].constraints.insert(prefix.steps[0].constraints.begin(),
                                           {"start", 15, {4, 7}});
        Engine path({{prefix}});
        calibrate(path);
        check(path.update(observation(path, 1300), 1300).empty(),
              "Interaction cannot bypass its ordered prerequisite");
        check(path.update(observation(path, 1400), 1400).empty(),
              "Holding sign alone cannot bypass prerequisite");
        path.update(observation(path, 1450, false, false), 1450);
        check(path.update(observation(path, 1500), 1500).empty(),
              "Dwell starts after prefix reaches cell");
        check(path.update(observation(path, 1600), 1600).size() == 1,
              "Ordered prefix then sign dwell fires");

        auto malformed = definition();
        malformed.steps[0].constraints[0].interaction.reset();
        bool rejected = false;
        try {
            validate(Configuration{{malformed}});
        } catch (const std::exception&) {
            rejected = true;
        }
        check(rejected, "Interaction requires explicit settings");
        std::cout << "Existing Interaction checks passed\n";
        for (bool mirrored : {false, true}) {
            auto motion = definition(600);
            motion.steps[0].constraints[0].interaction =
                Interaction{HandSide::Left, Gesture::V, 600};
            motion.keyboard = mig::ui::parse_binding("A _ A _ Ctrl + C");
            Configuration profile{{motion}};
            profile.controls.recalibrate = {Gesture::V,
                                            mirrored ? HandSide::Right : HandSide::Left};
            Engine standalone(profile);
            calibrate(standalone);
            GestureControls controls(profile.controls);
            mig::ui::KeyboardOutput output;
            std::vector<std::pair<int, bool>> sent;
            const auto send = [&](int key, bool release) {
                sent.emplace_back(key, release);
                return true;
            };
            for (int t = 1300; t <= 2700; t += 200) {
                auto frame = observation(standalone, t, mirrored);
                const int side = mirrored ? 1 : 0;
                for (int f = 0; f < 5; ++f) {
                    frame.fingers[side][f] = {f == 1 || f == 2 ? 1.f : 0.f, 1};
                }
                const auto events = standalone.update(frame, t);
                check(controls.update(frame, standalone.interaction_hands()) == 0,
                      "Occupied Interaction reserves its sign from global recalibration even while "
                      "latched");
                for (auto event : events) {
                    check(output.enqueue(standalone.configuration().motions[event.motion].keyboard),
                          "Purple-only event schedules keyboard action");
                }
                for (int tick = t; tick < t + 200; tick += 20) {
                    check(output.advance(tick, send, [](char16_t, bool) { return true; }),
                          "Action scheduler");
                }
            }
            check(output.empty() &&
                      std::count(sent.begin(), sent.end(), std::pair<int, bool>{65, false}) == 2 &&
                      std::count(sent.begin(), sent.end(), std::pair<int, bool>{17, false}) == 1,
                  "Purple-only 5fps hold emits one complete repeating keyboard sequence on both "
                  "sides");
        }
        std::cout
            << "Interaction dwell, geometry evidence, prefix, mirror, occlusion and latch passed\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
