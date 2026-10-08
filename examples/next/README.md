# Next.js camera examples

[English](README.md) | [Français](README.fr.md)

## Try it now

1. Extract the **complete built example package**, keeping its folders together.
2. Run `run.cmd` (Windows) or `sh run.sh` (Linux) in this folder of the built JavaScript examples archive; open `http://localhost:8820`.
3. Keep shoulders visible for calibration, lower your hands into the green region, then raise either wrist into yellow. Expect **Left/Right hand raised** once per wrist.

**Prerequisites:** Modern browser; complete built JavaScript examples archive with static `out/`. Source builds require Node.js and Next dependencies.

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

Import from `motion-input-grid/react`. Next.js requires a `'use client'` component. Serve the assets at `/mig/` over localhost or HTTPS. The package includes
WASM and models, so your application needs no C++ build. The checkout
commands below are for editing and rebuilding this example. See the
[npm guide](../../bindings/javascript/README.md).

## Run from the checkout

From the repository root, with Node.js 22.12+ and a compiled WASM engine:

```sh
npm ci
npm run prepare:javascript
npm run dev --workspace examples/next
```

Open `/` on the printed localhost URL for raised-hand detection, or `/profile`
for an initially empty configurator-profile importer. Start grants camera access.
Keep shoulders visible, lower hands and raise either wrist; accepted actions
appear on screen. Stop releases capture; Recalibrate resets progress. Invalid
profile imports preserve the previous configuration.

`app/page.jsx` and `app/profile/page.jsx` are server routes. They render the
client boundary `app/CameraExample.jsx`, which reuses React's example component
and the package hook. Browser URL access is guarded during prerendering; camera,
model and WASM initialization happen only after Start. The hook cleans up on
unmount/page exit. Shared CSS mirrors the video/overlay once while keeping text
normal. Profile files stay in the browser. `next.config.mjs` exports static pages.
Replace the shared component's `onAction` with application commands as needed.

After `npm run build:examples`, or archive extraction, run
`node examples/next/run.mjs`. Open `http://localhost:8820`; the server resolves
`/profile` to the exported page. Sources sit beside `out/`. No framework server
or npm install is needed for built output. Archive run.cmd/run.sh supplies
Node; a modern browser loads all MediaPipe/model assets locally. The archive includes shared React sources.

Next.js uses the same npm React adapter, with no separate recognition package.
The installed package and full-checkout workspace use identical imports. See
the [bootstrap and architecture guide](../../docs/integrations/javascript.md) and
[package preparation](../../bindings/javascript/README.md).

[Source walkthrough](../../docs/getting-started/examples.md).

## Project structure and MIG integration overview

`app/page.jsx` / `app/profile/page.jsx`: routes. `app/CameraExample.jsx`: client boundary. `../react/src/CameraExample.jsx`: shared hook UI. `../web/session.mjs` / `mig-tracker.mjs`: processing.

Framework/UI code owns rendering and user events. The named integration source owns configuration, observation submission, action retrieval and cleanup; it uses the public MIG API. Shared helpers are source references included with the archive.

## Walkthrough: initialization to shutdown

1. `app/CameraExample.jsx` is a client component that reuses the React viewer; server rendering does not open the camera.
2. `CameraExample.jsx` imports `useMIG` from `motion-input-grid/react`; its `assetBase` resolves local WASM/models relative to the page.
3. `useMIG({assetBase, profileMode, onAction})` creates and subscribes to `MIGSession`; video/canvas refs connect framework elements to the session.
4. The Start button calls `mig.start`. The session loads `default.json`, validates the WASM tracker, opens the camera after permission and supplies one fresh observation per frame.
5. `MIGTracker.update()` in the shared source submits buffers and retrieves action/ID pairs. `onAction` is the application callback; `mig.actions` is the UI snapshot.
6. `importProfile()` reads a bounded JSON file and calls `mig.importJSON`; invalid configuration preserves the engine. Change the callback to dispatch game commands.
7. The hook unsubscribes and disposes its session on unmount, including React StrictMode remounts. Stop closes stream tracks; Dispose also releases models/WASM.

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
