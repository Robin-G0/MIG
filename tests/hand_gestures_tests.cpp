#include <iostream>
#include <mig/core/session.hpp>
#include <mig/hands/fingers.hpp>
#include <stdexcept>
using namespace mig;
void check(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}
// Complete two-joint chains; a folded thumb's IP remains straight.
hands::Hand observed_hand(Vec2 wrist, Gesture gesture) {
    hands::Hand hand;
    hand.handedness_score = .9f;
    hand.points[0] = {wrist.x, wrist.y, 0};
    for (int finger = 0; finger < 5; ++finger) {
        const int base = finger == 0 ? 1 : 5 + (finger - 1) * 4;
        const bool open =
            gesture == Gesture::OpenPalm ||
            (gesture == Gesture::Thumb ? finger == 0
             : gesture == Gesture::OK  ? finger >= 2
                                       : gesture == Gesture::V && (finger == 1 || finger == 2));
        const float x = wrist.x + .005f * finger, y = wrist.y - .02f;
        hand.points[base] = {x, y, 0};
        hand.points[base + 1] = {x, y - .02f, 0};
        hand.points[base + 2] = {x, y + (open ? -.04f : 0.f), 0};
        hand.points[base + 3] = {x, y + (open ? -.06f : .02f), 0};
    }
    if (gesture == Gesture::OK) {
        hand.points[4] = hand.points[8];
    }
    return hand;
}
int main() {
    try {
        for (auto gesture :
             {Gesture::Thumb, Gesture::V, Gesture::OK, Gesture::OpenPalm, Gesture::Fist}) {
            for (int side = 0; side < 2; ++side) {
                Controls bindings;
                bindings.recalibrate = {gesture, HandSide(side)};
                GestureControls controls(bindings);
                Engine engine({});
                Frame frame;
                frame.aspect = 1;
                frame.points[11] = {{.65f, .45f}, 1};
                frame.points[12] = {{.35f, .45f}, 1};
                frame.points[15] = {{.7f, .65f}, 1};
                frame.points[16] = {{.3f, .65f}, 1};
                for (int t = 50; t <= 1800; t += 50) {
                    frame.timestamp_ms = frame.sequence = t;
                    engine.update(frame, t);
                }
                check(engine.grid().calibrated, "Initial calibration");
                bool fired = false;
                for (int t = 1850; t <= 3450; t += 50) {
                    frame.timestamp_ms = frame.sequence = t;
                    hands::Frame detected;
                    detected.timestamp_ms = t;
                    detected.sequence = t;
                    detected.count = 1;
                    detected.hands[0] = observed_hand(frame.points[15 + side].position, gesture);
                    const auto observations = hands::hand_observations(detected, frame);
                    frame.fingers = observations.fingers;
                    frame.hand_contacts = observations.contacts;
                    check(gesture_matches(frame.fingers[side], gesture, frame.hand_contacts[side]),
                          "Geometry must classify the authored sign");
                    if (gesture == Gesture::OK) {
                        detected.hands[0].points[4].x += .015f;
                        const auto separated = hands::hand_observations(detected, frame);
                        check(!gesture_matches(separated.fingers[side], gesture,
                                               separated.contacts[side]),
                              "OK needs touching tips, not just finger extension");
                    }
                    check(frame.fingers[side][0].confidence == 1, "Anatomical hand association");
                    const auto actions = controls.update(frame);
                    if (actions) {
                        check(!fired && actions == unsigned(ControlAction::Recalibrate),
                              "One recalibration command");
                        fired = true;
                        engine.recalibrate();
                        check(!engine.grid().calibrated && controls.suppressed(t),
                              "Gesture restarts calibration with recovery");
                    }
                }
                check(fired, "Thumb/V must reach application commands from hand geometry");
            }
        }
        for (int side = 0; side < 2; ++side) {
            Controls binding;
            binding.record_toggle = {Gesture::V, HandSide(side)};
            GestureControls recording_control(binding);
            Recorder recorder;
            Grid grid;
            grid.valid = grid.calibrated = true;
            grid.center = {.5f, .5f};
            grid.scale = .1f;
            bool started = false, stopped = false;
            for (int t = 0; t <= 3000; t += 200) {
                Frame body;
                body.timestamp_ms = body.sequence = t;
                body.aspect = 1;
                body.points[11] = {{.65f, .45f}, 1};
                body.points[12] = {{.35f, .45f}, 1};
                body.points[15] = {{.7f, .65f}, 1};
                body.points[16] = {{.3f, .65f}, 1};
                hands::Frame observation;
                observation.timestamp_ms = observation.sequence = t;
                observation.count = 1;
                auto& hand = observation.hands[0];
                hand = observed_hand(body.points[15 + side].position,
                                     t >= 1600 && t <= 1800 ? Gesture::Thumb : Gesture::V);
                body.fingers = hands::finger_observations(observation, body);
                const auto action = recording_control.update(body);
                if (action & unsigned(ControlAction::RecordToggle)) {
                    if (!recorder.active()) {
                        recorder.begin({15 + side});
                        started = true;
                    } else {
                        recorder.stop();
                        stopped = true;
                    }
                }
                if (!recording_control.suppressed(t)) {
                    recorder.sample(body, grid);
                }
            }
            check(started && stopped && !recorder.active() && !recorder.traces()[0].points.empty(),
                  "V geometry starts and stops recording at 5 fps with recovery and deliberate "
                  "release");
        }
        std::cout << "Hand geometry -> fingers -> Thumb/V -> recalibration/recording passed\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
