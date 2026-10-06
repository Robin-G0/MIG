# Vue camera examples

[English](README.md) | [Français](README.fr.md)

## Use in your application

```sh
npm install motion-input-grid
npx mig-copy-assets public/mig
```

Import from `motion-input-grid/vue`. Serve the assets at `/mig/` over localhost or HTTPS. The package includes
WASM and models, so your application needs no C++ build. The checkout
commands below are for editing and rebuilding this example. See the
[npm guide](../../bindings/javascript/README.md).

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

[Source walkthrough / explication du code](../../docs/getting-started/examples.md).
