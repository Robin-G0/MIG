# SDL2 camera example

[English](README.md) | [Français](README.fr.md)

## Try it now

1. Extract the **complete built example package**, keeping its folders together.
2. Run `mig-sdl2.exe` (Windows) or `./mig-sdl2` (Linux) from this folder.
3. Keep shoulders visible for calibration, lower your hands into the green region, then raise either wrist into yellow. Expect **Left/Right hand raised** once per wrist.

**Prerequisites:** Windows/Linux; prebuilt native examples archive; Windows VC++ runtime or Linux glibc 2.35+.

Desktop/browser built viewers target about **30–60 seconds after extraction**, with prerequisites installed; cold model loading depends on hardware. Editor and source builds have the longer setup described below. A source-only folder is not the prebuilt package.

## What this example demonstrates

Launch `mig-sdl2` (`mig-sdl2.exe` on Windows) without arguments.
The demo displays a mirrored camera, finger outlines and a prop for each wrist.
Follow the green region upward into yellow to display Left/Right hand raised.
`mig-sdl2-profile` starts empty and imports arbitrary configurator JSON via its
**Import profile** button or file drop. Actions appear in the window. Invalid
imports preserve the old profile; successful imports recalibrate. Both executables
are compiled from the same source with shared profile and recognition helpers.
Close the window to release the camera. No keys are injected.

Build with an installed MIG SDK:

```sh
cmake -S examples/sdl2 -B build/sdl2 -DCMAKE_PREFIX_PATH=/path/to/sdk
cmake --build build/sdl2 --config Release
```

If MIG is not installed, the build falls back to the full repository; bootstrap
native dependencies first. SDL2 and SDL2_ttf development libraries are needed.
Linux builds also need Qt6 Widgets for the profile picker; Windows uses its native
file dialog. The archive includes the redistributable DejaVu font and its license.
Run `tools/build-examples.ps1` on Windows or `tools/build-examples.sh` on Linux
for tested builds. The examples archives contain binaries and sources side by side.
The hidden `--smoke` option uses generated frames without opening a camera.

`demo::Source` owns capture/inference; `demo::consume` dispatches library events;
`draw_frame` mirrors the camera and coordinate overlays, including wrist props.
The camera texture and pixel buffers are reused between frames. The simple C++
demo captures synchronously; move Source ownership to a worker for independent
rendering latency in your game. See [standalone instructions](../README.md).

[Complete source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).

## Project structure and MIG integration overview

`main.cpp`: SDL window, event loop, camera texture and HUD. `../common/source.hpp`: camera/model ownership. `../common/recognition.hpp`: MIG processing and action dispatch.

Framework/UI code owns rendering and user events. The named integration source owns configuration, observation submission, action retrieval and cleanup; it uses the public MIG API. Shared helpers are source references included with the archive.

## Walkthrough: initialization to shutdown

1. Include `<mig/core/engine.hpp>` and `<mig/format/configuration.hpp>`; `main()` constructs `mig::Engine` from `mig::load_configuration(options.config)`.
2. `demo::Source::sample()` captures RGB and calls `Pose::infer()` once per new frame; it also supplies optional hands. These calls acquire observations, while `Engine` performs recognition.
3. `demo::consume()` calls `engine.update(body, body.timestamp_ms)` and resolves each event through `engine.configuration().motions[event.motion]`.
4. `draw_frame()` and `ActionHud` display actions. Replace this reaction with a game command; no desktop keys are injected.
5. The SDL close event exits the loop. C++ owners release textures, engine, camera and tasks; exceptions print an error and unwind those owners.

## MIG API used

`engine.update(frame, now_ms)`, `engine.configuration()`, `load_configuration()`, `Pose::infer()`.

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
