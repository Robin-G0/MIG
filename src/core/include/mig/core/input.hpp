#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mig {
enum class ConstraintType { Required, Forbidden, Trigger, Interaction };
inline bool is_trigger(ConstraintType type) noexcept {
    return type == ConstraintType::Trigger || type == ConstraintType::Interaction;
}
enum class Priority { High, Low };
enum class StepMode { Visited, Simultaneous, Ordered };
enum class CoordinateSpace { Body, Calibrated };
enum class HandSide { Left, Right };
enum class Finger { Thumb, Index, Middle, Ring, Pinky };
enum class FingerPose { Extended, Closed };
enum class Gesture { None, Thumb, V, OK, OpenPalm, Fist };
enum class KeyboardActionType { Chord, Text };
enum class ActionMode { SinglePress, Hold, Repeat };
inline constexpr std::array<std::string_view, 3> action_mode_names{"Single press", "Hold",
                                                                   "Repeat"};
struct KeyboardAction {
    KeyboardActionType type{KeyboardActionType::Chord};
    std::vector<int> keys;
    std::string text; // UTF-8, emitted as Unicode text rather than virtual keys.
    bool operator==(const KeyboardAction&) const = default;
};
std::u16string keyboard_text(std::string_view text);
void validate_keyboard(const std::vector<KeyboardAction>& sequence);
inline constexpr std::array<std::string_view, 6> gesture_names{"None", "Thumb",     "V",
                                                               "OK",   "Open palm", "Fist"};
inline constexpr int maximum_interaction_hold_ms = 60000;
struct Interaction {
    HandSide hand{HandSide::Left};
    Gesture gesture{Gesture::V};
    int hold_ms{};
    bool operator==(const Interaction&) const = default;
};
struct LandmarkDefinition {
    std::string_view name;
    int index;
    int mirror_index;
};
// Head is a derived midpoint of the ears, falling back to the nose.
inline constexpr std::array<LandmarkDefinition, 36> landmarks{{LandmarkDefinition{"head", 33, 33},
                                                               {"nose", 0, 0},
                                                               {"left_shoulder", 11, 12},
                                                               {"right_shoulder", 12, 11},
                                                               {"left_elbow", 13, 14},
                                                               {"right_elbow", 14, 13},
                                                               {"left_wrist", 15, 16},
                                                               {"right_wrist", 16, 15},
                                                               {"left_hand", 15, 16},
                                                               {"right_hand", 16, 15},
                                                               {"left_hip", 23, 24},
                                                               {"right_hip", 24, 23},
                                                               {"left_knee", 25, 26},
                                                               {"right_knee", 26, 25},
                                                               {"left_ankle", 27, 28},
                                                               {"right_ankle", 28, 27},
                                                               {"left_eye_inner", 1, 4},
                                                               {"left_eye", 2, 5},
                                                               {"left_eye_outer", 3, 6},
                                                               {"right_eye_inner", 4, 1},
                                                               {"right_eye", 5, 2},
                                                               {"right_eye_outer", 6, 3},
                                                               {"left_ear", 7, 8},
                                                               {"right_ear", 8, 7},
                                                               {"left_mouth", 9, 10},
                                                               {"right_mouth", 10, 9},
                                                               {"left_pinky", 17, 18},
                                                               {"right_pinky", 18, 17},
                                                               {"left_index", 19, 20},
                                                               {"right_index", 20, 19},
                                                               {"left_thumb", 21, 22},
                                                               {"right_thumb", 22, 21},
                                                               {"left_heel", 29, 30},
                                                               {"right_heel", 30, 29},
                                                               {"left_foot", 31, 32},
                                                               {"right_foot", 32, 31}}};
int landmark_index(std::string_view name);
std::string_view landmark_name(int index);
int mirrored_landmark(int index) noexcept;
struct FingerConstraint {
    HandSide hand{HandSide::Left};
    Finger finger{Finger::Index};
    FingerPose pose{FingerPose::Extended};
    int stable_ms{100}, grace_ms{150};
    bool operator==(const FingerConstraint&) const = default;
};
struct Cell {
    int x{}, y{}, width{1}, height{1};
    bool operator==(const Cell&) const = default;
};
struct SpatialConstraint {
    std::string id;
    int landmark{15};
    Cell cell;
    ConstraintType type{ConstraintType::Required};
    Priority priority{Priority::High};
    // Low Required/Trigger regions are alternatives for this High constraint.
    std::string tolerance_for;
    std::vector<FingerConstraint> fingers;
    // Positive numbers group alternative regions in an ordered landmark lane.
    // Zero retains independent constraints in array order.
    int order{};
    // Used only by Interaction cells; no timer state is persisted.
    std::optional<Interaction> interaction;
    bool operator==(const SpatialConstraint&) const = default;
};
struct Step {
    std::string id;
    StepMode mode{StepMode::Visited};
    std::vector<SpatialConstraint> constraints;
    std::vector<FingerConstraint> fingers;
    int hold_ms{};
    bool operator==(const Step&) const = default;
};
struct RecordedTrace {
    int landmark{15};
    std::vector<std::array<float, 2>> points;
    bool operator==(const RecordedTrace&) const = default;
};
struct ControlBinding {
    Gesture gesture{Gesture::None};
    HandSide hand{HandSide::Left};
    bool operator==(const ControlBinding&) const = default;
};
struct Controls {
    ControlBinding restart, recalibrate, record_toggle;
    int validation_ms{250};
    int post_gesture_delay_ms{1000};
    bool operator==(const Controls&) const = default;
};
struct FingerObservation {
    float extension{}, confidence{};
};
using FingerObservations = std::array<std::array<FingerObservation, 5>, 2>;
struct HandContact {
    float thumb_index_ratio{}, confidence{};
};
bool gesture_matches(const std::array<FingerObservation, 5>& hand, Gesture gesture,
                     HandContact contact = {}) noexcept;
bool gesture_observations_known(const std::array<FingerObservation, 5>& hand, Gesture gesture,
                                HandContact contact = {}) noexcept;
Gesture observed_gesture(const std::array<FingerObservation, 5>& hand,
                         HandContact contact = {}) noexcept;
} // namespace mig
