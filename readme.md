# Motion Input Grid (MIG)

[English](readme.md) | [Français](readme.fr.md)

MIG turns body and hand movements into named actions for applications and games.
Draw a movement in the [configurator](docs/guides/configurator.md), save its JSON profile, then run it in the
[controller](docs/guides/controller.md) or your own application. Use camera tracking or supply your own landmarks.

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
supplied landmarks; no ARM64 camera runtime is included. Python and browser libraries can be installed through pip and npm;
release archives provide offline installation alternatives.

## Install the libraries

```sh
python -m pip install motion-input-grid
npm install motion-input-grid
npx mig-copy-assets public/mig
```

Choose pip for Python (`from mig import Tracker`) or npm for the browser,
React, Vue and Next.js. See the guides for
[Python](bindings/python/README.md) · [JavaScript](bindings/javascript/README.md).

These packages are libraries. Desktop applications and the Python camera
runtime are separate downloads from
[Releases](https://github.com/Robin-G0/MIG/releases).

### Other installation options

Download the artifact matching your system from
[Releases](https://github.com/Robin-G0/MIG/releases).

| Use | Installation | Guide |
| --- | --- | --- |
| C++ / C ABI | Extract the `*-sdk` archive, add its path to `CMAKE_PREFIX_PATH` and use `find_package(MIG CONFIG REQUIRED)` | [SDK](docs/getting-started/cpp.md) |
| vcpkg | Extract `*-vcpkg-overlay.tar.gz`, then install `motion-input-grid` with `--overlay-ports`; the port is not yet in the main registry | [vcpkg port](ports/motion-input-grid/README.md) |
| Debian / Ubuntu | Download your architecture's `.deb` and install it with APT; installation by name requires a configured signed repository | [Debian / APT](docs/development/distribution.md#debian-and-signed-apt-hosting) |
| Godot | Extract the add-on ZIP at your project root | [Godot](integrations/godot/README.md) |
| Unity | Package Manager → **Add package from tarball**, using the Unity `.tgz` | [Unity UPM](integrations/unity/README.md) |
| Unreal | Extract the plugin ZIP into `Plugins`, then rebuild your C++ project | [Unreal](integrations/unreal/README.md) |

```sh
vcpkg install motion-input-grid --overlay-ports=/path/to/motion-input-grid-vcpkg-overlay
sudo apt install ./motion-input-grid_1.0.0_amd64.deb
```

Run the APT command in the download directory; choose the `arm64` file on ARM64.
The `.deb` and vcpkg port provide the SDK/engine without desktop applications or
a camera estimator. Godot, Unity and Unreal remain Preview integrations and
require a landmark provider.

For the configurator and controller, choose the `*-native` archive; for runnable
demos, choose the `*-examples` archive. Building from source is also supported:
[Windows](docs/getting-started/windows.md) · [Linux](docs/getting-started/linux.md).

## Desktop applications

The [configurator](docs/guides/configurator.md) creates profiles; the
[controller](docs/guides/controller.md) runs them. Their guides explain how to
launch each application and where to find its files.

For prebuilt applications, check [Releases](https://github.com/Robin-G0/MIG/releases)
for `motion-input-grid-<version>-windows-x64-native.zip` or
`motion-input-grid-<version>-linux-x64-native.tar.gz`. Extract the whole archive.
SDK archives and Python/npm/Debian packages provide libraries.
If no native archive has been published, follow the
[Windows](docs/getting-started/windows.md) or [Linux](docs/getting-started/linux.md) build guide.
The Windows `release` preset places both executables in `build/release/bin/`.

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

[Changelog](docs/development/CHANGELOG.md) · [Roadmap](docs/development/ROADMAP.md)
