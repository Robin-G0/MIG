# Unity desktop integration

[English](README.md) | [Français](README.fr.md)

## Try it now

1. Extract the **complete built example package**, keeping its folders together.
2. Install the matching runtime UPM package, add these scripts/profile to a Unity desktop project, attach **MigRaisedHands** and enable **UseSyntheticDemo**, then press Play.
3. Keep shoulders visible for calibration, lower your hands into the green region, then raise either wrist into yellow. Expect **Left/Right hand raised** once per wrist.

**Prerequisites:** Preview integration; Unity desktop editor and matching native plugin. Initial editor/package setup takes longer than one minute; no camera estimator is bundled.

Desktop/browser built viewers target about **30–60 seconds after extraction**, with prerequisites installed; cold model loading depends on hardware. Editor and source builds have the longer setup described below. A source-only folder is not the prebuilt package.

## What this example demonstrates

Requires a Unity C# desktop project. Install the matching [runtime UPM tarball](../../integrations/unity/README.md)
in Package Manager first. Copy `bindings/dotnet/SyntheticFrames.cs` and the three
example scripts into `Assets/Scripts`; the runtime package supplies `MigTracker`. Copy `Resources/MIG/raised-hands.json` into
`Assets/Resources/MIG/raised-hands.json`.
The runtime package supplies the platform-filtered native plugin. The positions-only build needs no MediaPipe models.
Assign a schema-v2 JSON TextAsset to the component's Profile field and connect OnAction.

Attach **MigRaisedHands** for the two-wrist demo, or **MigProfileInput** for arbitrary
profiles. Both show accepted actions in an on-screen panel. The profile variant
starts empty unless Profile is assigned; type a JSON file path into its panel and
click **Import JSON profile**, or call `ImportProfile(path)` from your own UI.
Failed validation preserves the old configuration; successful import recalibrates.

```csharp
var packet = MigTracker.Packet.Empty();
packet.TimestampMs = monotonicMilliseconds;
packet.Sequence = frameNumber;
packet.Aspect = cameraWidth / (float)cameraHeight;
packet.Body[15 * 8] = wristX;
packet.Body[15 * 8 + 1] = wristY;
packet.Body[15 * 8 + 3] = confidence;
component.SubmitFrame(packet);
```

Connect your estimator to `SubmitFrame()` on the main thread, once per new frame.
The example does not ship a Unity camera pose estimator; it accepts the provider's
unmirrored MediaPipe-style body/hand packets. `HandleAction()` invokes UnityEvent;
`OnDisable()` disposes the native engine. Optional world coordinates move the object
relative to the user's hips. `tracker.Active(index)` supports continuous Hold/Repeat
game semantics; OS keys are not injected. Windows/Linux desktop only, not IL2CPP
mobile/WebGL. See [Unity native plugin documentation](https://docs.unity3d.com/Manual/NativePlugins.html).

For a camera-free demonstration, also copy bindings/dotnet/SyntheticFrames.cs,
enable UseSyntheticDemo on MigRaisedHands. It produces one event per wrist.
The generic component's synthetic fixture remains intended for configs/default.json.
Disable synthetic mode when connecting your real estimator.

[Complete source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).

[Separate runtime package](../../integrations/unity/README.md).

## Project structure and MIG integration overview

`MigInput.cs`: direct MIG integration and Unity lifecycle. `MigRaisedHands.cs` / `MigProfileInput.cs`: initial mode selection. `Resources/MIG/raised-hands.json`: profile.

Framework/UI code owns rendering and user events. The named integration source owns configuration, observation submission, action retrieval and cleanup; it uses the public MIG API. Shared helpers are source references included with the archive.

## Walkthrough: initialization to shutdown

1. `MigInput.cs` imports `MotionInputGrid`; `OnEnable()` constructs `MigTracker` from the assigned TextAsset or Resources profile.
2. `SubmitFrame(packet)` accepts unmirrored body/hand observations with valid shoulders, monotonically increasing timestamps and sequences. Synthetic mode supplies demo packets in `Update()`.
3. `tracker.Update(ref packet, actionHandler)` recognizes and copies logical actions before callbacks. `HandleAction()` raises `OnAction` and updates the display.
4. `ImportProfile(path)` uses `ImportJson` for atomic validation/replacement. Bind `OnAction` to your game rather than desktop keyboard injection.
5. `OnDisable()` calls `tracker.Dispose()`. Do not call a disposed tracker or share it between threads.

## MIG API used

`MigTracker`, `Packet.Empty()`, `Update()`, `ImportJson()`, `TryCoordinate()`, `Dispose()`.

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
