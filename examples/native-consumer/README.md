# C++ native inference consumer

[English](README.md) | [Français](README.fr.md)

## Try it now

1. Extract the **complete built example package**, keeping its folders together.
2. Run `run.cmd` (Windows) or `sh run.sh` (Linux) in this folder of the prebuilt native examples archive.
3. Expect an inference sequence/capability line; blank RGB does not demonstrate a detected gesture.

**Prerequisites:** Native runtime/models are bundled in that archive. Source builds need CMake, C++20 and the native SDK. This is a blank-image inference check, with no camera or gesture demo.

Desktop/browser built viewers target about **30–60 seconds after extraction**, with prerequisites installed; cold model loading depends on hardware. Editor and source builds have the longer setup described below. A source-only folder is not the prebuilt package.

## What this example demonstrates

Requires the installed native SDK and bootstrapped MediaPipe runtime/models.
`demonstrate_inference(runtime, hands)` owns one estimator and performs inference
on blank RGB, demonstrating explicit hand capability and RAII cleanup.

```sh
cmake -S examples/native-consumer -B build/native-example -DCMAKE_PREFIX_PATH=/absolute/path/to/sdk
cmake --build build/native-example --config Release
build/native-example/mig-native-example /absolute/path/to/runtime --hands
```

Windows uses `Release/mig-native-example.exe`. Omit `--hands` for a body-only build.
Replace blank RGB with your capture frame and preserve width, height, timestamp and
sequence. A `Pose` and its borrowed results belong to one thread. Copy hand results
before the next inference call. For a complete capture loop, see `sdl2`/`sfml`.

[Complete source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).

## Project structure and MIG integration overview

`main.cpp`: `demonstrate_inference()` owns the estimator and blank RGB input; `CMakeLists.txt`: installed native SDK.

Framework/UI code owns rendering and user events. The named integration source owns configuration, observation submission, action retrieval and cleanup; it uses the public MIG API. Shared helpers are source references included with the archive.

## Walkthrough: initialization to shutdown

1. Include `<mig/native/pose.hpp>` and construct `mig::native::Pose(runtime_directory)` in `demonstrate_inference()`. The default model is Full.
2. `set_hands_enabled(hands)` explicitly controls hand inference; a build without hands rejects that request.
3. `infer(rgb, width, height, capture_ms, sequence)` returns body observations; blank RGB exercises ownership, not human detection.
4. Feed returned observations into a separate `mig::Engine::update()` in a real application. This consumer prints the returned sequence and capability, without generating movement actions.
5. Disable hands when no longer required. The scoped estimator closes tasks and releases its loaded runtime on return or exception. Keep estimator calls on one owning thread.

## MIG API used

`mig::native::Pose`, `set_hands_enabled()`, `infer()`, `hands_enabled()`.

## Configuration used

This inference-only consumer has no movement configuration.

## Reuse this in your project

Install the matching MIG package/SDK and retain the integration calls in the walkthrough. Copy the profile and required runtime assets with their licenses; use supplied tracking observations or the native camera adapter, as this example does. Replace the displayed/logged action with your application callback. Keep observations unmirrored, preserve camera aspect, submit one update per fresh frame, and provide missing observations when tracking is lost. Keep the tracker on one owner thread and preserve its cleanup hook. The application window, props and HUD are optional.

## Troubleshooting

- Missing native library/model or WASM: extract the whole built package and retain its runtime/assets folders. Check the prerequisite list; a source checkout needs the documented build.
- Camera unavailable: close other camera users; grant permission. Browser capture needs localhost or HTTPS. Engine previews need your own pose provider.
- No action: keep both shoulders visible, finish calibration, start in green, then raise into yellow. Paths require samples no more than 180 ms apart; very slow inference needs hardware profiling.
- Import fails: keep the error message and fix the schema/action it identifies. Failed validation preserves the old profile.
- Close/Stop releases owned resources; an in-flight native inference must finish before its worker can join.
