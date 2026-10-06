#include "mig_tracker.hpp"
#include <algorithm>
#include <godot_cpp/core/class_db.hpp>

namespace mig::godot_bridge {
using namespace godot;
void MigTrackerNative::_bind_methods() {
    ClassDB::bind_method(D_METHOD("open", "json"), &MigTrackerNative::open);
    ClassDB::bind_method(D_METHOD("import_json", "json"), &MigTrackerNative::import_json);
    ClassDB::bind_method(D_METHOD("close"), &MigTrackerNative::close);
    ClassDB::bind_method(D_METHOD("update", "timestamp_ms", "sequence", "aspect", "body", "hands",
                                  "hand_count", "hand_world_mask"),
                         &MigTrackerNative::update);
    ClassDB::bind_method(D_METHOD("event_action", "index"), &MigTrackerNative::event_action);
    ClassDB::bind_method(D_METHOD("event_id", "index"), &MigTrackerNative::event_id);
    ClassDB::bind_method(D_METHOD("active", "index"), &MigTrackerNative::active);
    ClassDB::bind_method(D_METHOD("coordinate", "joint", "system"), &MigTrackerNative::coordinate);
    ClassDB::bind_method(D_METHOD("reset", "recalibrate"), &MigTrackerNative::reset);
    ClassDB::bind_method(D_METHOD("get_error"), &MigTrackerNative::get_error);
}
MigTrackerNative::~MigTrackerNative() {
    close();
}
bool MigTrackerNative::valid_json(const String& json) {
    for (int64_t index = 0; index < json.length(); ++index) {
        if (json[index] == 0) {
            error_ = "Embedded NUL is not allowed.";
            return false;
        }
    }
    return true;
}
bool MigTrackerNative::result_ok(int result) {
    error_ = result < 0 ? String::utf8(mig_last_error()) : String();
    return result >= 0;
}
bool MigTrackerNative::open(const String& json) {
    if (!valid_json(json)) {
        return false;
    }
    if (mig_abi_version() != 1 || mig_packet_size() != sizeof(mig_packet)) {
        error_ = "Incompatible MIG C ABI.";
        return false;
    }
    auto replacement = mig_create(json.utf8().get_data());
    if (!replacement) {
        return result_ok(-1);
    }
    close();
    tracker_ = replacement;
    error_ = String();
    return true;
}
bool MigTrackerNative::import_json(const String& json) {
    if (!tracker_) {
        error_ = "Tracker is closed.";
        return false;
    }
    return valid_json(json) && result_ok(mig_load(tracker_, json.utf8().get_data()));
}
void MigTrackerNative::close() {
    mig_destroy(tracker_);
    tracker_ = nullptr;
}
int MigTrackerNative::update(int64_t timestamp_ms, int64_t sequence, double aspect,
                             const PackedFloat32Array& body, const PackedFloat32Array& hands,
                             int hand_count, int hand_world_mask) {
    if (!tracker_ || body.size() != 264 || hands.size() != 252 || sequence < 0 || hand_count < 0 ||
        hand_count > 2 || hand_world_mask < 0 || hand_world_mask > 3) {
        error_ = "Invalid packet dimensions, hand metadata or closed tracker.";
        if (tracker_) {
            mig_reset(tracker_, 0);
        }
        return -1;
    }
    packet_.timestamp_ms = timestamp_ms;
    packet_.sequence = static_cast<uint64_t>(sequence);
    packet_.aspect = static_cast<float>(aspect);
    packet_.hand_count = static_cast<uint32_t>(hand_count);
    packet_.hand_world_mask = static_cast<uint32_t>(hand_world_mask);
    std::copy_n(body.ptr(), 264, packet_.body);
    std::copy_n(hands.ptr(), 252, packet_.hands);
    const int count = mig_update(tracker_, &packet_);
    result_ok(count);
    return count;
}
String MigTrackerNative::event_action(int index) const {
    const auto value = tracker_ && index >= 0 ? mig_event_action(tracker_, index) : nullptr;
    return value ? String::utf8(value) : String();
}
String MigTrackerNative::event_id(int index) const {
    const auto value = tracker_ && index >= 0 ? mig_event_id(tracker_, index) : nullptr;
    return value ? String::utf8(value) : String();
}
bool MigTrackerNative::active(int index) const {
    return tracker_ && index >= 0 && mig_active(tracker_, index) > 0;
}
PackedFloat32Array MigTrackerNative::coordinate(int joint, int system) const {
    float point[4]{};
    PackedFloat32Array result;
    if (tracker_ && mig_coordinate(tracker_, joint, system, point) == 1) {
        result.resize(4);
        std::copy_n(point, 4, result.ptrw());
    }
    return result;
}
void MigTrackerNative::reset(bool recalibrate) {
    if (tracker_) {
        mig_reset(tracker_, recalibrate);
    }
}
String MigTrackerNative::get_error() const {
    return error_;
}
} // namespace mig::godot_bridge
