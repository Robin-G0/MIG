# Unity desktop integration

[English](README.md) | [Français](README.fr.md)

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
