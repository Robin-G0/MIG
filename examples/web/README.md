# Browser integration

[English](README.md) | [Français](README.fr.md)

## Try it now

1. Extract the **complete built example package**, keeping its folders together.
2. Run `run.cmd` (Windows) or `sh run.sh` (Linux) in the extracted JavaScript examples archive. Open `http://localhost:8820`.
3. Keep shoulders visible for calibration, lower your hands into the green region, then raise either wrist into yellow. Expect **Left/Right hand raised** once per wrist.

**Prerequisites:** Modern browser and complete built JavaScript examples archive; Node and local WASM/models are bundled. A raw checkout needs the build described below.

Desktop/browser built viewers target about **30–60 seconds after extraction**, with prerequisites installed; cold model loading depends on hardware. Editor and source builds have the longer setup described below. A source-only folder is not the prebuilt package.

## What this example demonstrates

Two variants share the same camera tracking: a raised-hand demo with visual
feedback and a JSON profile importer. Actions stay inside your application;
this example does not send desktop keyboard shortcuts.

## Use in your application

```sh
npm install motion-input-grid
npx mig-copy-assets public/mig
```

Import from `motion-input-grid`. Serve the assets at `/mig/` over localhost or HTTPS. The package includes
WASM and models, so your application needs no C++ build. The checkout
commands below are for editing and rebuilding this example. See the
[npm guide](../../bindings/javascript/README.md).

## Run from the checkout

Build with Emscripten, then package with `tools/package-distribution.ps1`, or copy
this directory together with `mig.mjs`, `mig.wasm`, `default.json`, `models/` and `vision/`.
Serve over localhost/HTTPS, never `file://`:

```sh
python3 -m http.server 8820 --directory distribution/web
```

Open `http://localhost:8820`. Start explicitly grants camera permission and loads
pinned MediaPipe Tasks Vision 0.10.35 from local assets. Prepare `vision/` with
`node tools/bootstrap-browser.mjs` before packaging. The JavaScript archive includes
Node launchers (`run.cmd` or `sh run.sh`), models, WASM and vision; no CDN is needed.

`index.html` is the raised-hands demo; `profile.html` starts empty and imports
arbitrary configurator JSON. Both use the same camera/tracker modules. Accepted
actions appear above the video with simultaneous events on separate lines; the
text panel is not mirrored. Invalid imports preserve the previous configuration;
successful imports clear old action feedback and restart calibration.
Packaging uses `common/raised-hands.json` as `default.json`, not `configs/default.json`.

```js
const tracker = await MIGTracker.create(profileJson);
tracker.update(poseResults, handResults, performance.now(), width / height,
    (action, inputId) => game.dispatch(action, inputId));
const wrist = tracker.coordinate(15, 2); // optional world metres, Y-up
tracker.dispose();
```

`mig-tracker.mjs` owns the WASM engine; `packets.mjs` copies fixed-size observations;
`models.mjs` handles model setup/partial-failure cleanup; `overlay.mjs` draws; and
`session.mjs` owns camera lifecycle for HTML, React, Vue and Next.js;
`camera.mjs` connects the session to DOM controls. HTML mirrors video/overlay
once. Feed empty detection results on loss. Reacquire buffer views after imports.
Callbacks are logical events; browsers cannot inject desktop keyboard keys.
Use `active(index)` and profile action settings for application Hold/Repeat scheduling.
Stop/page-close releases stream tracks and native model/engine resources.

[Complete source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).

## Project structure and MIG integration overview

`index.html` / `profile.html`, `camera.mjs`: browser controls. `session.mjs`: session lifecycle. `mig-tracker.mjs`: direct WASM calls. `packets.mjs`: observation buffers.

Framework/UI code owns rendering and user events. The named integration source owns configuration, observation submission, action retrieval and cleanup; it uses the public MIG API. Shared helpers are source references included with the archive.

## Walkthrough: initialization to shutdown

1. `camera.mjs` imports `MIGSession` from `session.mjs`; the session loads `mig-tracker.mjs`, models and overlay code from an explicitly resolved asset base.
2. `MIGSession.createTracker()` fetches `default.json`; `MIGTracker.create(json)` imports `mig.mjs`, constructs `new module.Tracker()` and validates/loads JSON before acquiring a camera.
3. Start calls `navigator.mediaDevices.getUserMedia()` after a user click. The session estimates one fresh video frame, and `packets.mjs` copies unmirrored body/hand observations into fixed buffers.
4. `MIGTracker.update()` calls the WASM tracker `update()` with monotonic milliseconds, sequence, aspect and hand metadata. It copies `eventAction()` / `eventId()` values before invoking application callbacks.
5. The session publishes action feedback; `camera.mjs` updates the text panel. Replace `onAction` with your game callback. Browser imports validate profiles but never inject desktop keyboard sequences.
6. Stop releases stream tracks. `dispose()` cancels the session generation, closes models and deletes the WASM engine; page teardown calls it. Failed startup/import leaves a useful error and releases partial resources.

## MIG API used

`MIGSession`, `MIGTracker.create()`, `Tracker.load()`, `Tracker.update()`, `eventAction()`, `eventId()`, `active()`, `dispose()`.

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
