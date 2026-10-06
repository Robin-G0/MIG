#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <mig/c/api.h>

namespace mig::godot_bridge {
class MigTrackerNative : public godot::RefCounted {
    GDCLASS(MigTrackerNative, godot::RefCounted)
public:
    ~MigTrackerNative() override;
    bool open(const godot::String& json);
    bool import_json(const godot::String& json);
    void close();
    int update(int64_t timestamp_ms, int64_t sequence, double aspect,
               const godot::PackedFloat32Array& body, const godot::PackedFloat32Array& hands,
               int hand_count, int hand_world_mask);
    godot::String event_action(int index) const;
    godot::String event_id(int index) const;
    bool active(int index) const;
    godot::PackedFloat32Array coordinate(int joint, int system) const;
    void reset(bool recalibrate);
    godot::String get_error() const;

protected:
    static void _bind_methods();

private:
    mig_tracker* tracker_{};
    mig_packet packet_{};
    godot::String error_;
    bool valid_json(const godot::String& json);
    bool result_ok(int result);
};
} // namespace mig::godot_bridge
