# Motion Input Grid (MIG)

[English](readme.md) | [Français](readme.fr.md)

MIG turns body and hand movements into named actions for applications and games.
Draw a movement in the configurator, save its JSON profile, then run it in the
controller or your own application. Use camera tracking or supply your own landmarks.

Landmarks → grid scaled to shoulder spacing → movement/sign constraints → action.
The same C++20 recognition engine serves every binding. Desktop keyboard output
supports Single press, Hold and Repeat and starts disabled.

Motion Input Grid (MIG) uses `motion-input-grid` as its distribution identifier.
Python keeps `import mig`; C++ keeps `find_package(MIG)` and `MIG::core`.

## Status and packages

MIG 1.0.0 is the first public release candidate. The core and C ABI contracts are
covered by regression and installed-consumer tests. Camera accuracy and engine
exports need separate checks; see the [support matrix](docs/reference/support.md).

| Ecosystem | Package / entry point | Maturity |
| --- | --- | --- |
| C++ / C ABI | SDK archive, `find_package(MIG)`, vcpkg overlay | Core Stable; vcpkg Beta |
| Python | Self-contained `motion-input-grid` wheel, `from mig import Tracker` | Beta |
| Browser / React / Vue / Next.js | `motion-input-grid`, same C++ engine in WASM | Beta |
| Debian / APT | `motion-input-grid`, signed-repository tooling | Beta |
| Godot / Unity / Unreal | Add-on ZIP / UPM tarball / Code Plugin ZIP | Preview |

Native camera applications target Windows/Linux x64. Linux ARM64 packages accept
supplied landmarks; no ARM64 camera runtime is included. Packages are prepared
locally; registry names and first uploads still require registration/review.

## Try it

The [standalone examples](examples/standalone.md) show a mirrored camera preview,
a prop following each wrist and feedback when a hand is raised. Extract the whole
matching examples archive, then run its binary or browser launcher.

For a source-based C++ smoke test, install an SDK and run:

```sh
cmake -S examples/sdk-consumer -B build/demo -DCMAKE_PREFIX_PATH=/path/to/sdk
cmake --build build/demo --config Release
```

Run the generated `mig-sdk-example` with `configs/default.json`; on Windows it is
under `build/demo/Release`. The example supplies synthetic positions and prints
`Game event: left_raise`. This checks recognition without a camera.

```python
from pathlib import Path
from mig import Tracker

profile = Path("examples/common/raised-hands.json").read_text()
with Tracker(None, profile) as tracker:
    print(tracker.export_json())
```

Install a matching wheel first. It includes the positions engine; camera viewers
need the separate native runtime. To feed observations and handle actions, follow
[bootstrap](docs/getting-started/bootstrap.md) and the [examples](examples/README.md).

[Documentation](docs/index.md) · [Architecture](docs/architecture/overview.md) ·
[Configuration](docs/reference/configuration.md) · [Distribution](docs/development/distribution.md) ·
[Contributing](CONTRIBUTING.md)

Licensed under [Apache-2.0](LICENSE). Redistributed dependencies retain their notices.
