# Unreal Engine 5 desktop integration

[English](README.md) | [Français](README.fr.md)

Copy this directory to `YourProject/Plugins/MigExample`. Copy an installed MIG SDK
into `ThirdParty/` beside the `.uplugin`, including `include/mig/c/api.h` and
`lib/mig-c.lib` + `bin/mig-c.dll` on Windows, or `lib/libmig-c.so` on Linux.
Regenerate project files and build the project. `MigExample.Build.cs` links the
C ABI and stages the shared library beside the packaged executable.

Add **MigRaisedHandsComponent** for the sample or **MigProfileInputComponent** for
arbitrary profiles. The sample JSON is staged from the plugin Content directory
as a loose file. Both create a UMG panel displaying all accepted actions, including
in Shipping builds; the importer has a path field and **Import JSON profile** button.
You can also call the Blueprint `ImportProfile(Path)` function. Invalid imports
preserve the old configuration; successful imports recalibrate.
Set ProfilePath to preload a loose JSON file, and bind
the Blueprint OnMotion event. Supply camera/provider observations on the game thread:

```cpp
mig_packet packet{};
packet.timestamp_ms = monotonic_ms;
packet.sequence = camera_sequence;
packet.aspect = width / float(height);
packet.body[15 * 8] = wrist_x;
packet.body[15 * 8 + 1] = wrist_y;
packet.body[15 * 8 + 3] = confidence;
component->SubmitFrame(packet);
```

`BeginPlay()` imports the profile; `SubmitFrame()` copies action strings before
Blueprint callbacks; `EndPlay()` destroys the engine. The host supplies unmirrored
body/hand packets and missing-data frames. This example does not bundle an Unreal
camera pose estimator. `mig_coordinate()` and `mig_active()` can add continuous
controls. Mapping metres to Unreal centimetres requires multiplication by 100 and
explicit axis mapping for your game's camera convention.
Keep the JSON loose (NonUFS); `FFileHelper` needs a real path. Windows/Linux only.
See [Epic library staging](https://dev.epicgames.com/documentation/en-us/unreal-engine/integrating-third-party-libraries-into-unreal-engine).

[Complete source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).

[Separate runtime package](../../integrations/unreal/README.md).
