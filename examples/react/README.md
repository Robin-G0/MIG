# React camera examples

[English](README.md) | [Français](README.fr.md)

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

[Source walkthrough / explication du code](../../docs/getting-started/examples.md).
