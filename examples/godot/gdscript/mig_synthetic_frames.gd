class_name MigSyntheticFrames
extends RefCounted


static func write_frame(body: PackedFloat32Array, hands: PackedFloat32Array,
        sequence: int, raised_hands: bool) -> void:
    body.fill(0.0)
    hands.fill(0.0)
    set_joint(body, 11, 0.65, 0.45)
    set_joint(body, 12, 0.35, 0.45)
    var row: float = 5.5 if sequence < 60 else (4.5 if sequence < 70 else 2.5)
    if raised_hands and sequence >= 60:
        row = maxf(1.5, 5.5 - (sequence - 59) / 5.0)
    var wrist_y: float = 0.45 + (row - 3.5) * 0.06
    set_joint(body, 15, 0.62, wrist_y)
    set_joint(body, 16, 0.38 if raised_hands else 0.25, wrist_y if raised_hands else 0.65)


static func set_joint(body: PackedFloat32Array, joint: int, x: float, y: float) -> void:
    var offset: int = joint * 8
    body[offset] = x
    body[offset + 1] = y
    body[offset + 3] = 1.0
