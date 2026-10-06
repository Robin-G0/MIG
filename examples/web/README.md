# Browser integration

[English](README.md) | [Français](README.fr.md)

## Use in your application

```sh
npm install motion-input-grid
npx mig-copy-assets public/mig
```

Import from `motion-input-grid`. Serve the assets at `/mig/` over localhost or HTTPS. The package includes
WASM and models, so your application needs no C++ build. The checkout
commands below are for editing and rebuilding this example. See the
[npm guide](../../bindings/javascript/README.md).

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
