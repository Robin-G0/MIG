# Godot 4 .NET desktop integration

[English](README.md) | [Français](README.fr.md)

## Try it now

1. Extract the **complete built example package**, keeping its folders together.
2. Use a Godot .NET desktop project, copy the bridge/scripts/profile as described below, attach **MigRaisedHands**, enable **UseSyntheticDemo** and run the scene.
3. Keep shoulders visible for calibration, lower your hands into the green region, then raise either wrist into yellow. Expect **Left/Right hand raised** once per wrist.

**Prerequisites:** Preview integration; Godot .NET, .NET SDK and a matching loose native ABI library. Initial setup exceeds one minute; no camera provider is bundled.

Desktop/browser built viewers target about **30–60 seconds after extraction**, with prerequisites installed; cold model loading depends on hardware. Editor and source builds have the longer setup described below. A source-only folder is not the prebuilt package.

## What this example demonstrates

Copy the scripts and profile from this csharp folder.
Use the Godot .NET editor and its supported .NET SDK. Copy
`../../../bindings/dotnet/MigTracker.cs`, `SyntheticFrames.cs` and the three example scripts
into your project. Copy `raised-hands.json` to `res://raised-hands.json`.
Attach **MigRaisedHands** for the demo or **MigProfileInput** to import arbitrary
profiles. Both add an on-screen action panel; the importer has a JSON file picker
and starts empty unless ProfilePath is assigned. Connect the MotionAction signal.
Failed imports preserve the previous configuration; successful imports recalibrate.
Editor-source archives already include the two shared bridge files in this folder.
Place `mig-c.dll` / `libmig-c.so` beside the executable or in the OS library search
path. For Linux development, launch with `LD_LIBRARY_PATH=/absolute/sdk/lib godot`.
Include the library as a loose file when exporting, not only inside the PCK.

```csharp
var packet = MigTracker.Packet.Empty();
packet.TimestampMs = monotonicMilliseconds;
packet.Sequence = cameraFrameNumber;
packet.Aspect = cameraWidth / (float)cameraHeight;
packet.Body[15 * 8] = wristX;
packet.Body[15 * 8 + 1] = wristY;
packet.Body[15 * 8 + 3] = confidence;
node.SubmitFrame(packet);
```

Supply packets from your own pose provider on the main thread; no estimator is
bundled in this game example. `_Ready()` imports JSON, `SubmitFrame()` dispatches
signals and optionally maps world Y-up XYZ into Godot's negative-Z-forward axes,
and `_ExitTree()` disposes the engine. Missing world data never moves the node.
Host actions are logical; use `Active(index)` for held game commands.
Desktop Windows/Linux only; Godot 4 C# web export is not supported.
See [Godot C# setup](https://docs.godotengine.org/en/stable/tutorials/scripting/c_sharp/c_sharp_basics.html).

For a camera-free demonstration, also copy ../../../bindings/dotnet/SyntheticFrames.cs,
enable UseSyntheticDemo on MigRaisedHands. It produces one event per wrist.
The generic component's synthetic fixture remains intended for configs/default.json.
Disable synthetic mode when connecting your real estimator.

[Complete source walkthrough](../../../docs/getting-started/examples.md) · [Bootstrap](../../../docs/getting-started/bootstrap.md).

## Project structure and MIG integration overview

`MigInput.cs`: direct MIG calls and Godot lifecycle. `MigRaisedHands.cs` / `MigProfileInput.cs`: mode selection. Shared `MigTracker.cs`: C ABI signatures/ownership.

Framework/UI code owns rendering and user events. The named integration source owns configuration, observation submission, action retrieval and cleanup; it uses the public MIG API. Shared helpers are source references included with the archive.

## Walkthrough: initialization to shutdown

1. `_Ready()` creates `MigTracker` and calls `ImportProfile()` for the requested JSON. The empty profile starts without actions.
2. `SubmitFrame(packet)` accepts provider observations on the main thread; `_Process()` optionally supplies synthetic demo packets.
3. `tracker.Update(ref packet, actionHandler)` runs recognition and copies actions before calling the handler.
4. The handler emits `MotionAction` and updates the Godot display. Connect the signal to a game action; `ImportJson()` validates atomically.
5. `_ExitTree()` disposes the native tracker. Ship the native library as a loose file outside the PCK.

## MIG API used

`MigTracker`, `Packet.Empty()`, `Update()`, `ImportJson()`, `Dispose()`.

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
