extends SceneTree

var failures: int = 0


func _initialize() -> void:
    run_tests.call_deferred()


func check(condition: bool, message: String) -> void:
    if not condition:
        failures += 1
        push_error(message)


func run_tests() -> void:
    var tracker := MigTrackerNative.new()
    var json := FileAccess.get_file_as_string("res://raised-hands.json")
    check(tracker.open(json), tracker.get_error())
    check(not tracker.import_json("{}"), "Invalid profile was accepted.")
    check(not tracker.open(json + String.chr(0) + "junk"), "Embedded NUL was accepted.")
    var body := PackedFloat32Array()
    var hands := PackedFloat32Array()
    body.resize(264)
    hands.resize(252)
    var actions: Array[String] = []
    for sequence in range(1, 91):
        MigSyntheticFrames.write_frame(body, hands, sequence, true)
        var count := tracker.update(sequence * 20, sequence, 1.0, body, hands, 0, 0)
        check(count >= 0, tracker.get_error())
        for index in range(count):
            actions.append(tracker.event_action(index))
            check(not tracker.event_id(index).is_empty(), "Missing input ID.")
    check(actions.count("left_raise") == 1, "Left wrist did not trigger exactly once.")
    check(actions.count("right_raise") == 1, "Right wrist did not trigger exactly once.")
    check(tracker.coordinate(15, 0).size() == 4, "Missing wrist coordinates.")
    check(tracker.coordinate(15, 2).is_empty(), "Missing world points became present.")
    check(tracker.update(2000, 100, 1.0, PackedFloat32Array(), hands, 0, 0) == -1,
        "Invalid packet dimensions were accepted.")
    check(not tracker.active(0), "Invalid packet left an action active.")
    tracker.close()
    tracker.close()
    check(not tracker.import_json(json), "Closed tracker accepted a profile.")
    check(tracker.open(json), "Tracker could not reopen.")
    tracker.close()
    await check_scene("res://raised_hands.tscn", json)
    await check_scene("res://profile.tscn", json)
    print("Godot GDScript bridge, recognition, imports, UI and lifecycle: %d failures." % failures)
    quit(0 if failures == 0 else 1)


func check_scene(path: String, json: String) -> void:
    var scene := load(path) as PackedScene
    var node := scene.instantiate() as MigInput
    root.add_child(node)
    await process_frame
    check(node.tracker != null, "Scene failed to initialize its tracker.")
    check(node.get_child_count() > 0, "Scene did not create its UI.")
    check(node.tracker.import_json(json), "Scene profile import failed.")
    var actions: Array[String] = []
    node.motion_action.connect(func(action: String, _input_id: String) -> void:
        actions.append(action)
        if actions.size() == 1:
            check(node.tracker.import_json(json), "Import during signal delivery failed."))
    for sequence in range(1, 91):
        MigSyntheticFrames.write_frame(node.demo_body, node.demo_hands, sequence, true)
        node.submit_frame(sequence * 20, sequence, 1.0, node.demo_body, node.demo_hands)
    check(actions.size() == 2, "Scene did not signal both wrist actions.")
    check(node.status.text.contains("left_raise") and node.status.text.contains("right_raise"),
        "Scene lost simultaneous action feedback.")
    node.queue_free()
    await process_frame
