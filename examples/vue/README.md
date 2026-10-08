# Vue camera examples

[English](README.md) | [Français](README.fr.md)

## Try it now

1. Extract the **complete built example package**, keeping its folders together.
2. Run `run.cmd` (Windows) or `sh run.sh` (Linux) in this folder of the built JavaScript examples archive; open `http://localhost:8820`.
3. Keep shoulders visible for calibration, lower your hands into the green region, then raise either wrist into yellow. Expect **Left/Right hand raised** once per wrist.

**Prerequisites:** Modern browser; complete built JavaScript examples archive. Source builds require Node.js and dependencies.

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

Import from `motion-input-grid/vue`. Serve the assets at `/mig/` over localhost or HTTPS. The package includes
WASM and models, so your application needs no C++ build. The checkout
commands below are for editing and rebuilding this example. See the
[npm guide](../../bindings/javascript/README.md).

## Run from the checkout

From the repository root, after building WASM, with Node.js 22.12+:

```sh
npm ci
npm run prepare:javascript
npm run dev --workspace examples/vue
```

Open the printed URL: `/` is the raised-hands demo; `/profile.html` starts empty
and runs imported configurator JSON. Start explicitly opens the camera. Keep
shoulders visible, lower hands and raise a wrist. Accepted actions appear on
screen, including simultaneous events. Stop releases the camera; Recalibrate
clears progress. Invalid profiles preserve the previous configuration.

`main.mjs` selects mode and mounts `CameraExample.vue`. The component calls the
package's `useMIG` composable, binds its video/canvas refs, reads the shallow
`state` ref and connects buttons/file import. Replace `onAction`'s console output
with application commands. The composable owns mount/unmount cleanup; inference
does not run in Vue rendering. Shared React example CSS mirrors only the preview
and supplies system themes/rounded controls. Vite's Vue plugin compiles the
component into both entry pages.

After `npm run build:examples` or archive extraction, run
`node examples/vue/run.mjs` and open `http://localhost:8820`. Built pages need
a browser, with bundled Node and local MediaPipe; no npm install. Source sits
beside `dist/`; the archive also includes the shared CSS. Imports resolve to the
installed package or the repository workspace.
See the [bootstrap and shared modules](../../docs/integrations/javascript.md) and
[package guide](../../bindings/javascript/README.md).

[Source walkthrough](../../docs/getting-started/examples.md).

## Project structure and MIG integration overview

`src/main.mjs`: Vue mount. `src/CameraExample.vue`: UI and MIG composable. `../../bindings/javascript/src/vue.mjs`: composable lifecycle. `../web/session.mjs` / `mig-tracker.mjs`: direct tracking calls.

Framework/UI code owns rendering and user events. The named integration source owns configuration, observation submission, action retrieval and cleanup; it uses the public MIG API. Shared helpers are source references included with the archive.

## Walkthrough: initialization to shutdown

1. `CameraExample.vue` imports `useMIG` from `motion-input-grid/vue`; its `assetBase` resolves local WASM/models relative to the page.
2. `useMIG({assetBase, profileMode, onAction})` creates and subscribes to `MIGSession`; video/canvas refs connect framework elements to the session.
3. The Start button calls `start`. The session loads `default.json`, validates the WASM tracker, opens the camera after permission and supplies one fresh observation per frame.
4. `MIGTracker.update()` in the shared source submits buffers and retrieves action/ID pairs. `onAction` is the application callback; `state.actions` is the UI snapshot.
5. `importProfile()` reads a bounded JSON file and calls `importJSON`; invalid configuration preserves the engine. Change the callback to dispatch game commands.
6. The composable unsubscribes and disposes its session on unmount, including Vue component teardown. Stop closes stream tracks; Dispose also releases models/WASM.

## MIG API used

`useMIG()`, `start()`, `stop()`, `recalibrate()`, `importJSON()`.

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
