# Unreal Engine 5 desktop integration

[English](README.md) | [Français](README.fr.md)

## Quick Start

1. Extract this example’s **standalone archive**; keep its contents together.
2. Install this plugin (its native SDK is bundled) in an Unreal desktop project, rebuild, attach **MigRaisedHandsComponent** and supply tracking packets to **SubmitFrame**.
3. With a configured pose provider, lower and raise either wrist; expect an action in the UMG panel.

**Prerequisites:** Preview integration; Unreal 5 editor, C++ toolchain and matching ABI library. Setup exceeds one minute; there is no bundled camera estimator or synthetic autoplay.

Desktop/browser built viewers target about **30–60 seconds after extraction**, with prerequisites installed; cold model loading depends on hardware. Editor and source builds have the longer setup described below. A source-only folder is not the prebuilt package.

## What this example demonstrates

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

## Folder walkthrough

`Source/MigExample/Private/MigInputComponent.cpp`: C ABI integration. Public component header: packet/event API. `MigExamplePanel.cpp`: UMG UI. `MigExample.Build.cs`: native linking/staging.

Framework/UI code owns rendering and user events. The named integration source owns configuration, observation submission, action retrieval and cleanup; it uses the public MIG API. Shared helpers are source references included with the archive.

## Code walkthrough

1. The component includes the MIG C ABI through `MigInputComponent.h`. `BeginPlay()` calls `mig_create()` and optionally imports the loose Content JSON.
2. `ImportProfile(path)` reads JSON and calls `mig_load()`. Failure reports `mig_last_error()` and preserves the prior profile.
3. Your provider calls `SubmitFrame(packet)` on the game thread. `mig_update()` recognizes unmirrored MediaPipe-indexed observations.
4. Copy `mig_event_action()` / `mig_event_id()` strings before broadcasting `OnMotion` to Blueprint callbacks, which may change the tracker.
5. `EndPlay()` calls `mig_destroy()` and clears ownership. The UMG panel is presentation; recognition remains in the MIG C++ engine.

## MIG API used

`mig_create()`, `mig_load()`, `mig_update()`, `mig_event_action()`, `mig_event_id()`, `mig_destroy()`.

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

## Standalone plugin dependencies

The individual archive contains `ThirdParty/include/mig/c`, the platform native
library (and Windows import library), the plugin descriptor, Source, Content
profile and licenses. Copy this folder to `YourProject/Plugins/MigExample`;
regenerate project files and build. An Unreal 5 desktop C++ project/compiler and
your own pose provider are external prerequisites. Editor compilation takes
longer than one minute; no camera estimator or models are bundled.
For a copied raw source folder, install the SDK into `ThirdParty/` as above.
