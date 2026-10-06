# Godot 4 .NET desktop integration

[English](README.md) | [Français](README.fr.md)

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
