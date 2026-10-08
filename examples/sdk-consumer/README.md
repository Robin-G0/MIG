# C++ positions-only consumer

[English](README.md) | [Français](README.fr.md)

## Try it now

1. Extract the **complete built example package**, keeping its folders together.
2. Run `run.cmd` (Windows) or `sh run.sh` (Linux) in this folder of the prebuilt native examples archive.
3. Expect `left_raise` on the console.

**Prerequisites:** Prebuilt executable: no camera or estimator. Building sources needs CMake 3.25+, C++20 and an installed/bundled SDK; compilation can exceed one minute.

Desktop/browser built viewers target about **30–60 seconds after extraction**, with prerequisites installed; cold model loading depends on hardware. Editor and source builds have the longer setup described below. A source-only folder is not the prebuilt package.

## What this example demonstrates

To install just the library and link it to your project, follow the
[C++ SDK / CMake guide](../../docs/getting-started/cpp.md). It covers archives, source builds,
`cmake --install` and generated Makefiles on Linux.

Requires CMake 3.25, C++20 and an installed MIG SDK with `MIG::core`/`MIG::format`.
No camera, models, Python or window system is needed.

```sh
cmake -S examples/sdk-consumer -B build/sdk-example -DCMAKE_PREFIX_PATH=/absolute/path/to/sdk
cmake --build build/sdk-example --config Release
build/sdk-example/mig-sdk-example configs/default.json
```

Visual Studio places the executable under `Release/`. The supplied profile emits
`left_raise`. `calibrate()` supplies stable synthetic shoulders; `demonstrate_path()`
walks one simple authored path; `dispatch_events()` shows the application callback.
Replace the synthetic frame construction with your estimator and retain
`engine.update(frame, frame.timestamp_ms)`. The synthetic walker is intentionally
not a simulator for arbitrary simultaneous/finger/Interaction profiles.

[Complete source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).

## Project structure and MIG integration overview

`main.cpp`: complete integration, including synthetic host observations and console dispatch. `CMakeLists.txt`: installed SDK links.

Framework/UI code owns rendering and user events. The named integration source owns configuration, observation submission, action retrieval and cleanup; it uses the public MIG API. Shared helpers are source references included with the archive.

## Walkthrough: initialization to shutdown

1. Include `<mig/core/engine.hpp>` and `<mig/format/configuration.hpp>` and create `mig::Engine(mig::load_configuration(path))` in `main()`.
2. `calibrate()` supplies valid synthetic shoulders, timestamps and sequences. Replace these synthetic observations with your tracking provider.
3. `demonstrate_path()` submits one landmark at the center of each authored region with `engine.update(frame, frame.timestamp_ms)`; it is a simple demonstration, not an arbitrary-profile simulator.
4. `dispatch_events()` resolves `event.motion` in `engine.configuration().motions` and prints its logical action. Replace this with application commands.
5. The scoped engine is destroyed on return or exception. There is no camera/model ownership or keyboard injection in this example.

## MIG API used

`mig::Engine`, `mig::load_configuration()`, `Engine::update()`, `Engine::configuration()`.

## Configuration used

This consumer takes a profile path (`configs/default.json` in the supplied launcher).

## Reuse this in your project

Install the matching MIG package/SDK and retain the integration calls in the walkthrough. Copy the profile and required runtime assets with their licenses; use supplied tracking observations or the native camera adapter, as this example does. Replace the displayed/logged action with your application callback. Keep observations unmirrored, preserve camera aspect, submit one update per fresh frame, and provide missing observations when tracking is lost. Keep the tracker on one owner thread and preserve its cleanup hook. The application window, props and HUD are optional.

## Troubleshooting

- Missing native library/model or WASM: extract the whole built package and retain its runtime/assets folders. Check the prerequisite list; a source checkout needs the documented build.
- Camera unavailable: close other camera users; grant permission. Browser capture needs localhost or HTTPS. Engine previews need your own pose provider.
- No action: keep both shoulders visible, finish calibration, start in green, then raise into yellow. Paths require samples no more than 180 ms apart; very slow inference needs hardware profiling.
- Import fails: keep the error message and fix the schema/action it identifies. Failed validation preserves the old profile.
- Close/Stop releases owned resources; an in-flight native inference must finish before its worker can join.
