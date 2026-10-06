# Godot 4 GDScript

[English](README.md) | [Français](README.fr.md)

Use the standard Godot 4.3+ desktop editor. `MigTrackerNative` is a GDExtension
wrapping MIG's C ABI; recognition stays in C++. No .NET installation is needed.

## Build and open

Install CMake 3.25+, a C++20 compiler and Python 3 for godot-cpp's generator:

```sh
git clone --depth 1 --branch godot-4.3-stable https://github.com/godotengine/godot-cpp build/godot-cpp
cmake -S examples/godot/gdscript/native -B build/godot-gdscript -DGODOT_CPP_DIR=/absolute/build/godot-cpp -DCMAKE_PREFIX_PATH=/absolute/mig-sdk
cmake --build build/godot-gdscript --config Release --parallel
```

Without an installed SDK, CMake builds the supplied-landmarks C API from a complete
MIG checkout, using its usual JSON dependency. No camera models are downloaded.
In an editor-source archive, use `-S gdscript/native` and its `sdk` folder.

Open `build/godot-gdscript/project/project.godot`. Run `raised_hands.tscn` for the
demo or `profile.tscn` for the importer. Enable **Use Synthetic Demo** on the
raised-hands node for one event per wrist without a camera. The importer starts
empty unless **Profile Path** is assigned. Its button opens a JSON picker.
Connect `motion_action(action, input_id)` for game actions. Invalid imports preserve
the current profile; successful imports recalibrate.

The generated `addons/mig/bin` contains the bridge and C API. Runtime packages
use the positions-only SDK and contain no MediaPipe models or camera provider.
Export native dependencies as loose files outside the PCK. Build matching
Windows/Linux x64 or ARM64 binaries. No estimator or web export is provided here.

## Supply observations

Call `submit_frame` on the main thread with reusable buffers:

```gdscript
var body := PackedFloat32Array()
var hands := PackedFloat32Array()
body.resize(264)
hands.resize(252)
body[15 * 8] = wrist_x
body[15 * 8 + 1] = wrist_y
body[15 * 8 + 3] = confidence
node.submit_frame(monotonic_ms, frame_number, camera_aspect, body, hands)
```

Supply shoulders and all points required by the profile, not just the wrist.
Body points use 33 groups of eight floats; hands use two groups of 21 points with
six floats each. See the [C ABI layout](../../../docs/reference/c-abi.md). Pass hand count
and world mask when supplying hand points. Coordinates remain anatomical and
unmirrored; mirror only your preview. Timestamps and sequences must advance.
World Y-up wrist coordinates move the node using Godot's negative-Z-forward axes;
missing world points never move it. Disable synthetic mode with a real provider.

`tracker.active(index)` exposes held conditions. Signals are accepted logical
events, not OS key presses. Import/update/reset/close belong to one owning thread;
`_exit_tree()` closes the tracker.

`mig_input.gd` owns UI/lifecycle; derived scripts select the two variants.
`mig_synthetic_frames.gd` supplies the fixture; `native/` delegates the build to
`integrations/godot/native`. The generated `addons/mig/mig.gdextension` declares
the library paths. See the separate [runtime add-on](../../../integrations/godot/README.md).

Headless regression checks after building:

```sh
godot --headless --path build/godot-gdscript/project --editor --import
godot --headless --path build/godot-gdscript/project --script res://tests/regression.gd
```

[Godot GDExtension setup](https://docs.godotengine.org/en/4.3/tutorials/scripting/gdextension/gdextension_cpp_example.html)
· [Source walkthrough](../../../docs/getting-started/examples.md).
