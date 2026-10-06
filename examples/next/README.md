# Next.js camera examples

[English](README.md) | [Français](README.fr.md)

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

[Source walkthrough / explication du code](../../docs/getting-started/examples.md).
