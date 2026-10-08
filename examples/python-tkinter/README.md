# Python/Tkinter camera examples

[English](README.md) | [Français](README.fr.md)

## Try it now

1. Extract the **complete built example package**, keeping its folders together.
2. Run `main.exe` (Windows) or `./main` (Linux) from this folder. The frozen archive includes Python, Pillow and Tk.
3. Keep shoulders visible for calibration, lower your hands into the green region, then raise either wrist into yellow. Expect **Left/Right hand raised** once per wrist.

**Prerequisites:** Prebuilt native examples archive; Windows VC++ runtime or Linux glibc 2.35+. Source execution additionally requires Python, Tk, Pillow and the native camera runtime.

Desktop/browser built viewers target about **30–60 seconds after extraction**, with prerequisites installed; cold model loading depends on hardware. Editor and source builds have the longer setup described below. A source-only folder is not the prebuilt package.

## What this example demonstrates

## Install the Python library

```sh
python -m pip install motion-input-grid pillow
```

The import remains `mig`. Tk is supplied separately by Python or the OS.
The wheel includes the positions engine; this camera example also needs
a camera-enabled native SDK and its models. Set `MIG_LIBRARY` to its
`mig-c.dll` / `libmig-c.so.1` and `MIG_RUNTIME` to its MediaPipe/model
directory when automatic discovery does not find them.
[Python installation](../../bindings/python/README.md).

Run `python3 examples/python-tkinter/main.py` for the raised-hands demo, or
`python3 examples/python-tkinter/profile.py` to import a configurator JSON profile.
Both start without arguments, display a mirrored camera and wrist-following props,
and show all accepted actions on screen. The importer has an **Import profile** button.
Invalid profiles keep the previous configuration running; successful imports recalibrate.

Install Python 3.10+, Tk and Pillow. On Ubuntu, install `python3-tk` and
`python3-pil.imagetk`. See [standalone setup](../standalone.md) for runtime discovery.
Installed `mig` is preferred, with the complete repository as fallback.
The inference worker owns the camera and tracker, including imports and teardown.
Closing the window joins the worker before destroying UI resources.

[Complete source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).

## Project structure and MIG integration overview

`main.py` / `profile.py`: entry points. `../common/tk_view.py`: Tk UI. `../common/python_source.py`: explicit MIG integration in `InputSource._run()`.

Framework/UI code owns rendering and user events. The named integration source owns configuration, observation submission, action retrieval and cleanup; it uses the public MIG API. Shared helpers are source references included with the archive.

## Walkthrough: initialization to shutdown

1. `python_source.py` imports `Packet` and `Tracker` from `mig`; installed bindings are preferred, with the bundled binding as fallback.
2. `InputSource._run()` reads the profile and creates `Tracker(library_path, profile_json)` in a context manager on its owner thread. `start_camera(runtime, camera_index)` explicitly creates the native camera/model adapter.
3. `tracker.poll_camera()` captures, estimates and recognizes once. Retrieve `tracker.events()` afterwards; do not call `update()` on the same packet again. Synthetic mode instead uses `tracker.update(packet)`.
4. Copy `tracker.camera_image()` before the next poll. The bounded mailbox carries observations, copied image bytes and accepted `(action, input_id)` tuples to the UI.
5. `Application.tick()` draws and calls `announce(events)`; replace that reaction with your application command. Import requests are handled on the same tracker thread, and invalid JSON preserves its configuration.
6. `Application.close()` calls `InputSource.close()`, sets the stop event and joins the worker before destroying Tk. Leaving the context manager closes the tracker and native tasks.

## MIG API used

`Tracker(...)`, `start_camera()`, `poll_camera()`, `events()`, `update()`, `import_json()`, `close()`.

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
