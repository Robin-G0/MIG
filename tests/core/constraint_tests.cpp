#include <algorithm>
#include <iostream>
#include <mig/core/engine.hpp>
#include <stdexcept>
using namespace mig;
void check(bool ok, const char* message) {
    if (!ok) {
        throw std::runtime_error(message);
    }
}
Frame sample(std::int64_t time) {
    Frame frame;
    frame.timestamp_ms = frame.sequence = time;
    frame.aspect = 1;
    frame.points[11] = {{0.65f, 0.45f}, 1};
    frame.points[12] = {{0.35f, 0.45f}, 1};
    frame.points[0] = {{0.5f, 0.3f}, 1};
    frame.points[15] = {{0.7f, 0.65f}, 1};
    frame.points[16] = {{0.3f, 0.65f}, 1};
    return frame;
}
void calibrate(Engine& engine) {
    for (int t = 50; t <= 1250; t += 50) {
        engine.update(sample(t), t);
    }
    engine.restart();
    engine.set_test_mode(true);
}
void place(Frame& frame, const Grid& grid, int landmark, Vec2 position) {
    frame.points[landmark] = {grid.metric(position), 1};
}
Motion input() {
    Motion motion;
    motion.id = motion.name = motion.action = "generic";
    motion.space = CoordinateSpace::Calibrated;
    Step start{"guard"};
    start.constraints = {{"guard", 16, {2, 5}}};
    Step finish{"finish"};
    finish.constraints = {{"head", 33, {4, 0}}, {"hit", 16, {2, 3}, ConstraintType::Trigger}};
    motion.steps = {start, finish};
    return motion;
}
int main() {
    try {
        Motion rows;
        rows.id = "raise_anywhere";
        rows.space = CoordinateSpace::Calibrated;
        rows.mirror = true;
        Step row_step{"rows", StepMode::Ordered};
        for (int number = 1; number <= 2; ++number) {
            for (int x = 1; x <= 3; ++x) {
                SpatialConstraint region;
                region.id = std::to_string(number) + "_" + std::to_string(x);
                region.landmark = 16;
                region.cell = {x, 7 - number * 2};
                region.order = number;
                row_step.constraints.push_back(region);
            }
        }
        rows.steps = {row_step};
        const auto rejects_group = [&](Motion invalid) {
            bool rejected = false;
            try {
                Engine invalid_engine({{invalid}});
            } catch (const std::exception&) {
                rejected = true;
            }
            check(rejected, "Malformed order groups must be rejected");
        };
        auto invalid_rows = rows;
        invalid_rows.steps[0].constraints[0].order = 0;
        rejects_group(invalid_rows);
        invalid_rows = rows;
        invalid_rows.steps[0].constraints[0].type = ConstraintType::Forbidden;
        rejects_group(invalid_rows);
        invalid_rows = rows;
        invalid_rows.steps[0].constraints[0].type = ConstraintType::Trigger;
        rejects_group(invalid_rows);
        invalid_rows = rows;
        invalid_rows.steps[0].mode = StepMode::Simultaneous;
        rejects_group(invalid_rows);
        invalid_rows = rows;
        invalid_rows.steps[0].constraints[0].order = 1025;
        rejects_group(invalid_rows);
        for (bool mirrored_side : {false, true}) {
            for (int column = 1; column <= 3; ++column) {
                Engine row_engine({{rows}});
                calibrate(row_engine);
                auto first = sample(1300);
                const int wrist = mirrored_side ? 15 : 16;
                const float x = mirrored_side ? 9.f - (column + .5f) : column + .5f;
                place(first, row_engine.reference_grid(), wrist, {x, 5.5f});
                check(row_engine.update(first, 1300).empty(), "Row 1 is only the prerequisite");
                const auto& accepted = row_engine.progress(0, mirrored_side).constraints;
                check(std::count(accepted.begin(), accepted.end(), ConstraintStatus::Validated) ==
                          1,
                      "Only the visited alternative should be highlighted");
                auto second = sample(1350);
                place(second, row_engine.reference_grid(), wrist, {x, 3.5f});
                check(row_engine.update(second, 1350).size() == 1,
                      "Any column in each numbered row completes the movement, including Mirror");
            }
        }
        Engine backwards({{rows}});
        calibrate(backwards);
        auto wrong = sample(1300);
        place(wrong, backwards.reference_grid(), 16, {2.5f, 3.5f});
        check(backwards.update(wrong, 1300).empty(), "Row 2 cannot skip row 1");
        wrong = sample(1350);
        place(wrong, backwards.reference_grid(), 16, {2.5f, 5.5f});
        check(backwards.update(wrong, 1350).empty(), "Reverse travel does not complete the input");
        auto reordered_rows = rows;
        std::reverse(reordered_rows.steps[0].constraints.begin(),
                     reordered_rows.steps[0].constraints.end());
        Engine reordered({{reordered_rows}});
        calibrate(reordered);
        auto reordered_frame = sample(1300);
        place(reordered_frame, reordered.reference_grid(), 16, {2.5f, 5.5f});
        check(reordered.update(reordered_frame, 1300).empty(),
              "Explicit order overrides array order");
        reordered_frame = sample(1350);
        place(reordered_frame, reordered.reference_grid(), 16, {2.5f, 3.5f});
        check(reordered.update(reordered_frame, 1350).size() == 1,
              "Numbered groups remain ordered after rearranging serialized constraints");
        auto tolerated_rows = rows;
        tolerated_rows.steps[0].constraints[0].fingers = {
            {HandSide::Left, Finger::Index, FingerPose::Extended}};
        SpatialConstraint row_tolerance{"row_tolerance", 16,   {4, 5}, ConstraintType::Required,
                                        Priority::Low,   "1_2"};
        tolerated_rows.steps[0].constraints.push_back(row_tolerance);
        Engine tolerated({{tolerated_rows}});
        calibrate(tolerated);
        auto tolerated_frame = sample(1300);
        place(tolerated_frame, tolerated.reference_grid(), 16, {4.5f, 5.5f});
        tolerated.update(tolerated_frame, 1300);
        check(tolerated.progress(0).constraints[0] != ConstraintStatus::Validated &&
                  tolerated.progress(0).constraints[1] == ConstraintStatus::Validated &&
                  tolerated.progress(0).constraints[6] == ConstraintStatus::Validated,
              "A group's alternative uses its own fingers and tolerance, not the first region's");
        tolerated_frame = sample(1350);
        place(tolerated_frame, tolerated.reference_grid(), 16, {3.5f, 3.5f});
        check(tolerated.update(tolerated_frame, 1350).size() == 1,
              "Tolerance on an alternative preserves numbered progression");
        for (auto& item : rows.steps[0].constraints) {
            if (item.order == 2) {
                item.type = ConstraintType::Trigger;
            }
        }
        Engine row_trigger({{rows}});
        calibrate(row_trigger);
        auto row_frame = sample(1300);
        place(row_frame, row_trigger.reference_grid(), 16, {3.5f, 5.5f});
        row_trigger.update(row_frame, 1300);
        row_frame = sample(1350);
        place(row_frame, row_trigger.reference_grid(), 16, {3.5f, 3.5f});
        check(row_trigger.update(row_frame, 1350).size() == 1 &&
                  row_trigger.progress(0).constraints[5] == ConstraintStatus::Triggered &&
                  row_trigger.progress(0).constraints[3] == ConstraintStatus::Missing,
              "Multiple yellow alternatives form one Trigger and highlight the actual contact");
        Engine engine({{input()}});
        calibrate(engine);
        auto frame = sample(1300);
        place(frame, engine.reference_grid(), 16, {2.5f, 5.5f});
        check(engine.update(frame, 1300).empty() && engine.progress(0).step == 1, "Ordered steps");
        frame = sample(1350);
        place(frame, engine.reference_grid(), 0, {4.5f, 0.5f});
        place(frame, engine.reference_grid(), 16, {2.5f, 3.5f});
        check(engine.update(frame, 1350).size() == 1, "Multi-landmark trigger before suffix");
        check(engine.progress(0).constraints.back() == ConstraintStatus::Triggered,
              "Trigger highlight");
        frame.timestamp_ms = frame.sequence = 1400;
        check(engine.update(frame, 1400).empty(), "No repeated activation");
        engine.restart();
        check(engine.grid().valid && engine.progress(0).step == 0 && !engine.progress(0).triggered,
              "Restart without calibration");

        auto mirrored = input();
        mirrored.mirror = true;
        Engine mirror({{mirrored}});
        calibrate(mirror);
        frame = sample(1300);
        place(frame, mirror.reference_grid(), 15, {6.5f, 5.5f});
        mirror.update(frame, 1300);
        frame = sample(1350);
        place(frame, mirror.reference_grid(), 0, {4.5f, 0.5f});
        place(frame, mirror.reference_grid(), 15, {6.5f, 3.5f});
        check(mirror.update(frame, 1350).size() == 1 && mirror.progress(0, true).triggered,
              "Mirror coordinates and landmark swap");

        auto forbidden = input();
        forbidden.constraints = {{"avoid", 0, {4, 0}, ConstraintType::Forbidden}};
        Engine blocked({{forbidden}});
        calibrate(blocked);
        frame = sample(1300);
        place(frame, blocked.reference_grid(), 16, {2.5f, 5.5f});
        blocked.update(frame, 1300);
        frame = sample(1350);
        place(frame, blocked.reference_grid(), 0, {4.5f, 0.5f});
        place(frame, blocked.reference_grid(), 16, {2.5f, 3.5f});
        check(blocked.update(frame, 1350).empty() && blocked.progress(0).failed &&
                  blocked.progress(0).constraints[0] == ConstraintStatus::Forbidden,
              "Forbidden blocks trigger");

        Motion jump;
        jump.id = "jump";
        Step rise{"rise", StepMode::Simultaneous};
        rise.constraints = {
            {"head", 33, {4, -2, 1, 3}}, {"left", 11, {6, 1}}, {"right", 12, {2, 1}}};
        jump.steps = {rise};
        for (const bool crouch : {false, true}) {
            auto posture = jump;
            if (crouch) {
                posture.id = "crouch";
                for (auto& constraint : posture.steps[0].constraints) {
                    constraint.cell.y += 4;
                }
            }
            Engine body({{posture}});
            calibrate(body);
            frame = sample(1300);
            const float y = crouch ? 5.5f : 1.5f;
            place(frame, body.reference_grid(), 11, {6.5f, y});
            place(frame, body.reference_grid(), 12, {2.5f, y});
            place(frame, body.reference_grid(), 0, {4.5f, crouch ? 3.5f : -0.5f});
            check(body.update(frame, 1300).size() == 1,
                  "Generic Jump/Crouch against calibrated anchor");
        }

        auto fingers = input();
        fingers.steps[1].fingers = {{HandSide::Right, Finger::Index, FingerPose::Extended}};
        Engine hand({{fingers}});
        calibrate(hand);
        frame = sample(1300);
        place(frame, hand.reference_grid(), 16, {2.5f, 5.5f});
        hand.update(frame, 1300);
        frame = sample(1350);
        place(frame, hand.reference_grid(), 0, {4.5f, 0.5f});
        place(frame, hand.reference_grid(), 16, {2.5f, 3.5f});
        frame.fingers[1][1] = {1, 1};
        check(hand.update(frame, 1350).empty(), "One finger frame is insufficient");
        for (int t : {1400, 1450}) {
            frame.timestamp_ms = frame.sequence = t;
            const auto events = hand.update(frame, t);
            check(events.size() == (t == 1450 ? 1u : 0u), "Finger temporal stability");
        }
        auto tolerance = input();
        tolerance.steps[0].constraints.push_back(
            {"margin", 16, {3, 5}, ConstraintType::Required, Priority::Low, "guard"});
        Engine tolerant({{tolerance}});
        calibrate(tolerant);
        frame = sample(1300);
        place(frame, tolerant.reference_grid(), 16, {3.5f, 5.5f});
        tolerant.update(frame, 1300);
        // Start must include the tolerance alternative too.
        check(tolerant.progress(0).step == 1, "Low tolerance starts and validates matching High");
        auto low_forbidden = forbidden;
        low_forbidden.constraints[0].priority = Priority::Low;
        Engine low_blocked({{low_forbidden}});
        calibrate(low_blocked);
        frame = sample(1300);
        place(frame, low_blocked.reference_grid(), 16, {2.5f, 5.5f});
        place(frame, low_blocked.reference_grid(), 0, {4.5f, 0.5f});
        check(low_blocked.update(frame, 1300).empty() && low_blocked.progress(0).failed,
              "Low Forbidden also invalidates");

        Motion parallel;
        parallel.id = "parallel";
        Step ordered{"parallel_step", StepMode::Ordered};
        ordered.constraints = {{"left_a", 15, {6, 5}},
                               {"left_b", 15, {6, 3}},
                               {"right_a", 16, {2, 5}},
                               {"right_b", 16, {2, 3}}};
        parallel.steps = {ordered};
        Engine independent({{parallel}});
        calibrate(independent);
        frame = sample(1300);
        place(frame, independent.reference_grid(), 15, {6.5f, 5.5f});
        place(frame, independent.reference_grid(), 16, {2.5f, 5.5f});
        check(independent.update(frame, 1300).empty(), "Parallel start");
        frame.timestamp_ms = frame.sequence = 1350;
        place(frame, independent.reference_grid(), 16, {2.5f, 3.5f});
        check(independent.update(frame, 1350).empty(), "Right can advance before left");
        frame.timestamp_ms = frame.sequence = 1400;
        place(frame, independent.reference_grid(), 15, {6.5f, 3.5f});
        check(independent.update(frame, 1400).size() == 1, "Independent ordered landmark lanes");

        parallel.steps[0].mode = StepMode::Simultaneous;
        parallel.steps[0].constraints = {{"left", 15, {6, 3}}, {"right", 16, {2, 3}}};
        Engine together({{parallel}});
        calibrate(together);
        frame = sample(1300);
        place(frame, together.reference_grid(), 15, {6.5f, 3.5f});
        place(frame, together.reference_grid(), 16, {2.5f, 5.5f});
        check(together.update(frame, 1300).empty(), "Simultaneous cannot accumulate one hand");
        frame.timestamp_ms = frame.sequence = 1350;
        place(frame, together.reference_grid(), 15, {6.5f, 5.5f});
        place(frame, together.reference_grid(), 16, {2.5f, 3.5f});
        check(together.update(frame, 1350).empty(), "Alternating hands is not simultaneous");
        frame.timestamp_ms = frame.sequence = 1400;
        place(frame, together.reference_grid(), 15, {6.5f, 3.5f});
        check(together.update(frame, 1400).size() == 1, "Simultaneous shared frame");
        Motion prefix;
        prefix.id = "prefix";
        Step path{"prefix_step", StepMode::Ordered};
        path.constraints = {{"start", 16, {2, 5}},
                            {"trigger", 16, {2, 3}, ConstraintType::Trigger},
                            {"suffix", 16, {2, 1}}};
        prefix.steps = {path};
        Engine early({{prefix}});
        calibrate(early);
        frame = sample(1300);
        place(frame, early.reference_grid(), 16, {2.5f, 5.5f});
        early.update(frame, 1300);
        frame.timestamp_ms = frame.sequence = 1350;
        place(frame, early.reference_grid(), 16, {2.5f, 3.5f});
        check(early.update(frame, 1350).size() == 1 &&
                  early.progress(0).constraints.back() == ConstraintStatus::Missing,
              "Ordered Trigger fires without same-step suffix");
        // The same logical action must work with either anatomical hand.
        for (int authored : {15, 16}) {
            Motion bilateral;
            bilateral.id = "bilateral";
            bilateral.mirror = true;
            const int start_x = authored == 15 ? 6 : 2;
            bilateral.steps = {{"path", StepMode::Ordered}};
            bilateral.steps[0].constraints = {
                {"start", authored, {start_x, 5}},
                {"finish", authored, {start_x, 3}, ConstraintType::Trigger}};
            for (int mirrored_variant = 0; mirrored_variant < 2; ++mirrored_variant) {
                Engine both({{bilateral}});
                calibrate(both);
                auto attempt = sample(1300);
                const int observed_landmark =
                    mirrored_variant ? mirrored_landmark(authored) : authored;
                const float x = mirrored_variant ? 9.f - (start_x + .5f) : start_x + .5f;
                place(attempt, both.reference_grid(), observed_landmark, {x, 5.5f});
                both.update(attempt, 1300);
                attempt.timestamp_ms = attempt.sequence = 1350;
                place(attempt, both.reference_grid(), observed_landmark, {x, 3.5f});
                check(both.update(attempt, 1350).size() == 1 &&
                          both.progress(0, mirrored_variant != 0).triggered,
                      "Mirror recognises either hand with reflected cells as the same input");
            }
        }
        Motion repeat;
        repeat.id = "repeat";
        repeat.mirror = true;
        repeat.cooldown_ms = 500;
        repeat.steps = {{"path", StepMode::Ordered}};
        repeat.steps[0].constraints = {{"start", 16, {2, 5}},
                                       {"finish", 16, {2, 3}, ConstraintType::Trigger}};
        Engine cooldown({{repeat}});
        calibrate(cooldown);
        cooldown.set_test_mode(false);
        const auto move = [&](int t, Vec2 point) {
            auto observed = sample(t);
            place(observed, cooldown.reference_grid(), 16, point);
            return cooldown.update(observed, t).size();
        };
        check(move(1300, {2.5f, 5.5f}) == 0 && move(1350, {2.5f, 3.5f}) == 1,
              "Cooldown begins after firing");
        check(move(1400, {8.5f, 8.5f}) == 0 && move(1450, {2.5f, 5.5f}) == 0 &&
                  move(1500, {2.5f, 3.5f}) == 0,
              "Cooldown suppresses another complete attempt");
        for (int t = 1550; t <= 1850; t += 50) {
            move(t, {8.5f, 8.5f});
        }
        check(move(1900, {2.5f, 5.5f}) == 0 && move(1950, {2.5f, 3.5f}) == 1,
              "A fresh attempt fires after optional cooldown");

        repeat.mirror = false;
        repeat.cooldown_ms = 0;
        Engine unlimited({{repeat}});
        repeat.max_duration_ms = 1000;
        Engine timed({{repeat}});
        calibrate(unlimited);
        calibrate(timed);
        for (int t = 1300; t <= 3500; t += 50) {
            auto observed = sample(t);
            place(observed, unlimited.reference_grid(), 16, {2.5f, 5.5f});
            unlimited.update(observed, t);
            timed.update(observed, t);
        }
        auto late = sample(3550);
        place(late, unlimited.reference_grid(), 16, {2.5f, 3.5f});
        check(unlimited.update(late, 3550).size() == 1 && timed.update(late, 3550).empty() &&
                  timed.progress(0).failed,
              "No default duration; explicit Pro time limit retained");
        std::cout << "Generic constraints, posture, mirror, forbidden, finger stability and "
                     "diagnostics passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
