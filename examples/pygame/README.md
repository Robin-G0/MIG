# Pygame camera example

[English](README.md) | [Français](README.fr.md)

## Install the Python library

```sh
python -m pip install motion-input-grid pillow pygame
```

The import remains `mig`. Tk is supplied separately by Python or the OS.
The wheel includes the positions engine; this camera example also needs
a camera-enabled native SDK and its models. Set `MIG_LIBRARY` to its
`mig-c.dll` / `libmig-c.so.1` and `MIG_RUNTIME` to its MediaPipe/model
directory when automatic discovery does not find them.
[Python installation](../../bindings/python/README.md).

Run from the complete checkout or extracted examples archive:

```sh
python3 -m pip install motion-input-grid pillow pygame
python3 examples/pygame/main.py
python3 examples/pygame/profile.py
```

Keep shoulders visible for one second, lower the hands, then raise either hand
through the green rows into yellow. A mirrored prop follows each wrist, and the
terminal prints which hand was raised. Recognition uses MIG, not Python thresholds.
The window also shows both accepted actions. `profile.py` starts empty and imports
arbitrary configurator JSON using its button or file drop. Install Tk for the picker.
Invalid imports keep the previous profile; successful imports restart calibration.
The camera opens on launch. Close joins the owner thread before shutting down SDL;
the event loop stops drawing immediately on QUIT. MediaPipe stderr telemetry or
feedback warnings are upstream diagnostics, not a Python traceback.

Installed `mig` is preferred; `bindings/python` is the checkout fallback.
Native runtime, models and DLL/SO are detected from the built checkout or archive.
See [standalone instructions](../standalone.md) for dependencies, custom SDK
selection, package contents and the camera-free smoke check.

`python_source.InputSource` owns tracking on a worker and publishes one latest
RGB frame/landmark packet plus bounded events. `announce` handles logical events;
`wrists` mirrors coordinates for display. Replace the action callback or prop drawing
to integrate your application. Native camera pixels come from `Tracker.camera_image`.

[Complete source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).
