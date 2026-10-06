# Motion Input Grid (MIG) browser package

[English](README.md) | [Français](README.fr.md)

## Install from npm

In your application (Node.js 22.12+), run:

```sh
npm install motion-input-grid
npx mig-copy-assets public/mig
```

The package includes WASM, models, MediaPipe, types and licenses. Serve
`public/mig` at `/mig/` over localhost or HTTPS. React and Vue use this
same package; Next.js uses the React adapter. Install your chosen framework
in your application.

```js
import { MIGSession } from 'motion-input-grid';
import { useMIG } from 'motion-input-grid/react'; // React / Next.js
// import { useMIG } from 'motion-input-grid/vue'; // Vue
```

Use only the import matching your application. In PowerShell, use
`npm.cmd` and `npx.cmd` if the execution policy blocks `.ps1` scripts.

`motion-input-grid` provides a browser camera session, React hook and Vue
composable backed by the existing C++/WASM engine. React and Vue are optional
peer dependencies; Next.js uses the React adapter.

## Alternative: build the package

Build the WASM engine first, then run from the full checkout:

```sh
npm ci
npm run prepare:javascript
npm pack --workspace motion-input-grid --pack-destination build/releases
```

The tarball includes WASM, model files, runtime modules, declarations and licenses.
Install that local tarball into your application, then copy its runtime assets:

```sh
npm install /path/to/motion-input-grid-1.0.0.tgz
npx --package motion-input-grid mig-copy-assets public/mig
```

## Use the package

Pass the public asset URL to `useMIG` or `MIGSession`. The default is `/mig/`;
applications hosted under a subpath should pass their actual URL. Keep all runtime
files together. The camera opens only after `start(video, canvas)` or the example's
Start button. Serve over localhost or HTTPS.

```js
import { MIGSession } from 'motion-input-grid';

const session = new MIGSession({
    assetBase: new URL('/mig/', location.href).href,
    onAction: ({ action, id }) => console.log(action, id)
});
const unsubscribe = session.subscribe(state => updateStatus(state.status));
await session.start(videoElement, canvasElement);
await session.importJSON(configuratorJson);
unsubscribe();
session.dispose();
```

`subscribe` immediately supplies a snapshot, then receives status changes and
accepted actions. The action list holds the most recent accepted frame, including
simultaneous events; it is feedback rather than live held-state information.
`stop` releases the camera and resets recognition, retaining models for restart.
`dispose` also closes models and the engine. It is idempotent. Failed profile
validation leaves the active configuration intact. `recalibrate` resets progress.
The session's optional `onFrame(session)` callback reads `coordinate(index,
system)` for imperative animation/XYZ feedback without reactive updates; system
0 is normalized image XYZ, 1 world metres, 2 world metres with Y up. Missing
coordinates return `null`. Keep callbacks short to avoid delaying inference.

React exports `useMIG` from `motion-input-grid/react`. Attach its `video` and
`canvas` refs and bind `start`, `stop`, `recalibrate`, `importJSON`. Vue exports the
same operations from `motion-input-grid/vue` and a shallow `state` ref. React
options `assetBase`/`profileMode` recreate the session when changed; Vue options
are fixed for that component's lifetime. Both dispose on unmount/page exit.
`onAction` receives `{ action, id }`; use it for your application's commands.

Inference stays outside reactive state. Only new video frames are processed,
hand inference follows `tracking.hands`, and coordinates are mirrored only by
the preview's CSS. No desktop keyboard injection occurs. MediaPipe Tasks Vision
0.10.35 is included in `runtime/vision` and copied with the other public assets.
Start loads local files; no CDN request or consumer dependency installation is needed.

See the [React](https://github.com/Robin-G0/MIG/blob/main/examples/react/README.md), [Vue](https://github.com/Robin-G0/MIG/blob/main/examples/vue/README.md)
and [Next.js](https://github.com/Robin-G0/MIG/blob/main/examples/next/README.md) examples and the
[JavaScript bootstrap guide](https://github.com/Robin-G0/MIG/blob/main/docs/integrations/javascript.md).
