# Godot 4 GDScript

[English](README.md) | [Français](README.fr.md)

## Try it now

1. Extract the **complete built example package**, keeping its folders together.
2. Build/install the matching GDExtension as described below, open the staged project, run `raised_hands.tscn` and enable **Use Synthetic Demo** on its node.
3. Keep shoulders visible for calibration, lower your hands into the green region, then raise either wrist into yellow. Expect **Left/Right hand raised** once per wrist.

**Prerequisites:** Preview integration; Godot 4.3+ desktop editor and matching native bridge. Initial C++/godot-cpp setup exceeds one minute; no camera provider is bundled.

Desktop/browser built viewers target about **30–60 seconds after extraction**, with prerequisites installed; cold model loading depends on hardware. Editor and source builds have the longer setup described below. A source-only folder is not the prebuilt package.

## What this example demonstrates

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

## Project structure and MIG integration overview

`mig_input.gd`: MIG lifecycle and game UI. `mig_raised_hands.gd`: initial mode. Native `MigTrackerNative` bridge calls the C ABI; scene files configure nodes.

Framework/UI code owns rendering and user events. The named integration source owns configuration, observation submission, action retrieval and cleanup; it uses the public MIG API. Shared helpers are source references included with the archive.

## Walkthrough: initialization to shutdown

1. `_ready()` creates `MigTrackerNative` and opens a validated empty profile; `import_profile()` loads the selected JSON with `import_json()`.
2. `submit_frame()` accepts unmirrored body/hand buffers, timestamps, sequence and aspect. `_process()` optionally supplies synthetic packets.
3. The native bridge calls `mig_update()` and copies its action strings. The node emits `motion_action(action, input_id)` and updates status.
4. Connect that signal to your game command. Import failures display `get_error()` and retain the active configuration.
5. `_exit_tree()` closes the tracker; the native reference also owns the C handle. Keep calls on the main thread.

## MIG API used

`MigTrackerNative.new()`, `open()`, `import_json()`, `update()`, `close()`.

## Configuration used

The raised-hand demo uses `raised-hands.json` (served as `default.json` in browser assets): one broad Required zone `[-9,3,27,3]`, then a Trigger zone `[-9,1,27,2]` for each wrist. Import mode starts empty and validates schema-v2 JSON before replacement. Step order retains the upward movement; an isolated pose in yellow cannot fire.

## Reuse this in your project

Install the matching MIG package/SDK and retain the integration calls in the walkthrough. Copy the profile and required runtime assets with their licenses; use supplied tracking observations or the native camera adapter, as this example does. Replace the displayed/logged action with your application callback. Keep observations unmirrored, preserve camera aspect, submit one update per fresh frame, and provide missing observations when tracking is lost. Keep the tracker on one owner thread and preserve its cleanup hook. The application window, props and HUD are optional.

## Troubleshooting

- Missing native library/model or WASM: extract the whole built package and retain its runtime/assets folders. Check the prerequisite list; a source checkout needs the documented build.
- Camera unavailable: close other camera users; grant permission. Browser capture needs localhost or HTTPS. Engine previews need your own pose provider.
- No action: keep both shoulders visible, finish calibration, start in green, then raise into yellow. Paths require samples no more than 180 ms apart; very slow inference needs hardware profiling.
- Import fails: keep the error message and fix the schema/action it identifies. Failed validation preserves the old profile.
- Close/Stop releases owned resources; an in-flight native inference must finish before its worker can join.
