# Motion Input Grid (MIG) Python package

[English](README.md) | [Français](README.fr.md)

Python 3.10+ access to MIG's C++ recognition engine through standard-library ctypes.
No Python inference dependency is required.

Install with `python -m pip install motion-input-grid` after the package is published,
or `python -m pip install ./bindings/python` from a checkout.

Release wheels include the positions-only C ABI library and its licenses.
Windows uses `mig-c.dll`; Linux x64/ARM64 uses `libmig-c.so.1`. No models or camera
inference are included. Pass `None` to use the bundled library, or an explicit
path to select an external camera-enabled SDK. A source archive includes the
canonical C++ engine and JSON headers; rebuilding it requires CMake 3.25+ and a
C++20 compiler. Linux source builds use GCC and its runtime copyright notices.

```python
from pathlib import Path
from mig import Packet, Tracker

profile = Path("profile.json").read_text(encoding="utf-8")
with Tracker(None, profile) as tracker:
    frame = Packet(timestamp_ms=20, sequence=1, aspect=16 / 9)
    frame.set_body(11, 0.65, 0.45)
    frame.set_body(12, 0.35, 0.45)
    frame.set_body(15, 0.4, 0.6)
    for action, input_id in tracker.update(frame):
        print(action, input_id)
```

Use increasing timestamps in milliseconds and increasing sequence numbers.
Calibration requires roughly one second with both shoulders visible.
Positions use unmirrored anatomical MediaPipe indices; missing points have zero
confidence. Keep each tracker on one owning thread and close it explicitly.

`start_camera(runtime_directory)` and `poll_camera()` optionally use a native
camera-enabled SDK plus its MediaPipe runtime and models. Poll already updates
recognition; consume `events()` afterward without submitting the frame twice.
Action callbacks are logical events; keyboard injection is the host's choice.

See the repository's [examples](https://github.com/Robin-G0/MIG/tree/main/examples)
and [ABI contract](https://github.com/Robin-G0/MIG/blob/main/docs/reference/c-abi.md) for body
and hand XYZ packets, capture ownership, Pygame and Tkinter integration.

[Complete source walkthrough](https://github.com/Robin-G0/MIG/blob/main/docs/getting-started/examples.md) · [Bootstrap](https://github.com/Robin-G0/MIG/blob/main/docs/getting-started/bootstrap.md).
