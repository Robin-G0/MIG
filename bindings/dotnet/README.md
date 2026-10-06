# C# bridge

[English](README.md) | [Français](README.fr.md)

`MigTracker.cs` wraps ABI version 1 of `mig-c` for Unity and Godot .NET.
Copy it into the project and stage the matching native library for the editor
and each export architecture. No NuGet package is published by this repository.

Construct a tracker with schema-v2 JSON. Reuse `MigTracker.Packet.Empty()` arrays,
populate fresh observations and call `Update(ref packet, callback)` on one owning
thread. The callback receives copied action/input strings. `TryCoordinate`
requires a buffer of at least four floats and returns freshness/confidence.
`ImportJson` validates atomically and resets recognition on success. Dispose
on scene disable/exit; do not update the same handle concurrently.

Packet body/hand layout, coordinate spaces, borrowed lifetimes and capability
limits are in [C API](../../docs/reference/c-abi.md). Start with
[Unity](../../examples/unity/README.md) or [Godot](../../examples/godot/README.md).
These components demonstrate both imported profiles and raised-wrist feedback;
an actual observation provider is still needed outside synthetic mode.
