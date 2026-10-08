class_name MigInput
extends Node3D

signal motion_action(action: String, input_id: String)

@export_file("*.json") var profile_path: String = ""
@export var use_synthetic_demo: bool = false

var tracker: MigTrackerNative
var status: Label
var demo_sequence: int = 0
var demo_time: float = 0.0
var demo_body: PackedFloat32Array = PackedFloat32Array()
var demo_hands: PackedFloat32Array = PackedFloat32Array()


func is_raised_hands() -> bool:
    return false


func _ready() -> void:
    create_interface()
    # Own one native tracker for this node; release it in _exit_tree().
    tracker = MigTrackerNative.new()
    if not tracker.open('{"schema_version":2,"tracking":{"hands":true},"inputs":[]}'):
        show_error(tracker.get_error())
        tracker = null
        return
    var initial_profile: String = "res://raised-hands.json" if is_raised_hands() else profile_path
    if not initial_profile.is_empty():
        import_profile(initial_profile)
    demo_body.resize(264)
    demo_hands.resize(252)


func create_interface() -> void:
    var layer := CanvasLayer.new()
    add_child(layer)
    var panel := PanelContainer.new()
    panel.position = Vector2(12, 12)
    layer.add_child(panel)
    var stack := VBoxContainer.new()
    panel.add_child(stack)
    status = Label.new()
    status.text = "Lower your hands, then raise either wrist." if is_raised_hands() else \
        "Import a profile, then keep shoulders visible."
    if not is_raised_hands():
        create_import_button(layer, stack)
    stack.add_child(status)


func create_import_button(layer: CanvasLayer, stack: VBoxContainer) -> void:
    var dialog := FileDialog.new()
    dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
    dialog.access = FileDialog.ACCESS_FILESYSTEM
    dialog.filters = PackedStringArray(["*.json ; MIG profiles"])
    layer.add_child(dialog)
    dialog.file_selected.connect(import_profile)
    var button := Button.new()
    button.text = "Import JSON profile"
    button.pressed.connect(func() -> void: dialog.popup_centered_ratio())
    stack.add_child(button)


func import_profile(path: String) -> void:
    if tracker == null:
        show_error("Tracker is unavailable.")
        return
    var profile := FileAccess.open(path, FileAccess.READ)
    if profile == null:
        show_error("Cannot open MIG profile: " + path)
        return
    var json := profile.get_as_text()
    profile.close()
    if not tracker.import_json(json):
        show_error(tracker.get_error())
        return
    status.text = "Profile imported. Recalibrating."


func submit_frame(timestamp_ms: int, sequence: int, aspect: float,
        body: PackedFloat32Array, hands: PackedFloat32Array,
        hand_count: int = 0, hand_world_mask: int = 0) -> void:
    if tracker == null:
        return
    var count := tracker.update(timestamp_ms, sequence, aspect, body, hands,
        hand_count, hand_world_mask)
    if count < 0:
        show_error(tracker.get_error())
        return
    if count > 0:
        status.text = ""
    var events: Array[PackedStringArray] = []
    for index in range(count):
        events.append(PackedStringArray([tracker.event_action(index), tracker.event_id(index)]))
    for event in events:
        handle_action(event[0], event[1])
        if not is_inside_tree() or tracker == null:
            return
    var wrist := tracker.coordinate(15, 2)
    if wrist.size() == 4:
        position = Vector3(wrist[0], wrist[1], -wrist[2])


func handle_action(action: String, input_id: String) -> void:
    if not status.text.is_empty():
        status.text += "\n"
    status.text += "%s (input %s)" % [action, input_id]
    print("MIG %s: %s" % [input_id, action])
    motion_action.emit(action, input_id)


func _process(delta: float) -> void:
    if not use_synthetic_demo or tracker == null or demo_sequence >= 90:
        return
    demo_time += delta
    if demo_time < 0.02:
        return
    demo_time = 0.0
    demo_sequence += 1
    MigSyntheticFrames.write_frame(demo_body, demo_hands, demo_sequence, is_raised_hands())
    submit_frame(demo_sequence * 20, demo_sequence, 1.0, demo_body, demo_hands)


func show_error(message: String) -> void:
    status.text = message
    push_error(message)


func _exit_tree() -> void:
    if tracker != null:
        tracker.close()
        tracker = null
