# React camera examples

[English](README.md) | [Français](README.fr.md)

## Try it now

1. Extract the **complete built example package**, keeping its folders together.
2. Run `run.cmd` (Windows) or `sh run.sh` (Linux) in this folder of the built JavaScript examples archive; open `http://localhost:8820`.
3. Keep shoulders visible for calibration, lower your hands into the green region, then raise either wrist into yellow. Expect **Left/Right hand raised** once per wrist.

**Prerequisites:** Modern browser; complete built JavaScript examples archive. No npm installation is needed to run built pages. Source builds require Node.js and dependencies.

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

Import from `motion-input-grid/react`. Serve the assets at `/mig/` over localhost or HTTPS. The package includes
WASM and models, so your application needs no C++ build. The checkout
commands below are for editing and rebuilding this example. See the
[npm guide](../../bindings/javascript/README.md).

## Run from the checkout

From the repository root, with Node.js 22.12+ and the compiled WASM engine:

```sh
npm ci
npm run prepare:javascript
npm run dev --workspace examples/react
```

Open the printed localhost URL. `/` detects raised wrists and displays Left/Right
hand raised. `/profile.html` starts empty and imports configurator JSON, displaying
the accepted actions and input IDs. Start grants camera access; Stop releases it;
Recalibrate clears recognition progress. Keep shoulders visible for calibration,
lower hands, then raise them. The preview and paper planes mirror once.

`src/main.jsx` chooses the mode and mounts in StrictMode. `CameraExample.jsx`
uses the package's `useMIG` hook, attaches preview refs, renders feedback and
passes file text to `importJSON`. Validation errors preserve the previous profile.
Its `onAction` currently logs events: replace it with your application's command.
`style.css` supplies rounded controls, system light/dark theme and mirrored preview.
`vite.config.mjs` builds both HTML pages. Both reuse one component and session.

Run compiled pages using `node examples/react/run.mjs` after
`npm run build:examples`, or from the extracted JavaScript examples archive.
Open `http://localhost:8820`; no npm install is needed for built pages. A modern browser is required. Archive run.cmd/run.sh uses bundled Node;
MediaPipe and models load locally.
Sources stay beside `dist/` in the archive.

Use the installed npm package by name in your project; repository workspaces
resolve the same import to `bindings/javascript`. See the
[complete bootstrap and shared-code explanation](../../docs/integrations/javascript.md) and
[package setup](../../bindings/javascript/README.md).

[Source walkthrough](../../docs/getting-started/examples.md).

## Project structure and MIG integration overview

`src/main.jsx`: React mount and mode selection. `src/CameraExample.jsx`: UI and MIG hook. `../../bindings/javascript/src/react.mjs`: hook lifecycle. `../web/session.mjs` and `../web/mig-tracker.mjs`: tracking and direct WASM calls.

Framework/UI code owns rendering and user events. The named integration source owns configuration, observation submission, action retrieval and cleanup; it uses the public MIG API. Shared helpers are source references included with the archive.

## Walkthrough: initialization to shutdown

1. `CameraExample.jsx` imports `useMIG` from `motion-input-grid/react`; its `assetBase` resolves local WASM/models relative to the page.
2. `useMIG({assetBase, profileMode, onAction})` creates and subscribes to `MIGSession`; video/canvas refs connect framework elements to the session.
3. The Start button calls `mig.start`. The session loads `default.json`, validates the WASM tracker, opens the camera after permission and supplies one fresh observation per frame.
4. `MIGTracker.update()` in the shared source submits buffers and retrieves action/ID pairs. `onAction` is the application callback; `mig.actions` is the UI snapshot.
5. `importProfile()` reads a bounded JSON file and calls `mig.importJSON`; invalid configuration preserves the engine. Change the callback to dispatch game commands.
6. The hook unsubscribes and disposes its session on unmount, including React StrictMode remounts. Stop closes stream tracks; Dispose also releases models/WASM.

## MIG API used

`useMIG()`, `start()`, `stop()`, `recalibrate()`, `importJSON()`; shared `Tracker.update()` / action queries.

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
