# Documentation

[English](index.md) | [Français](index.fr.md)

## Install the libraries

```sh
python -m pip install motion-input-grid
npm install motion-input-grid
npx mig-copy-assets public/mig
```

Choose pip for Python (`from mig import Tracker`) or npm for the browser,
React, Vue and Next.js. See the guides for
[Python](../bindings/python/README.md) · [JavaScript](../bindings/javascript/README.md).

These packages are libraries. Desktop applications and the Python camera
runtime are separate downloads from
[Releases](https://github.com/Robin-G0/MIG/releases).

### Other installation options

Download the artifact matching your system from
[Releases](https://github.com/Robin-G0/MIG/releases).

| Use | Installation | Guide |
| --- | --- | --- |
| Install and link the C++ SDK / CMake | [C++ installation](getting-started/cpp.md) |
| C++ / C ABI | Extract the `*-sdk` archive, add its path to `CMAKE_PREFIX_PATH` and use `find_package(MIG CONFIG REQUIRED)` | [SDK](../docs/getting-started/cpp.md) |
| vcpkg | Extract `*-vcpkg-overlay.tar.gz`, then install `motion-input-grid` with `--overlay-ports`; the port is not yet in the main registry | [vcpkg port](../ports/motion-input-grid/README.md) |
| Debian / Ubuntu | Download your architecture's `.deb` and install it with APT; installation by name requires a configured signed repository | [Debian / APT](../docs/development/distribution.md#debian-and-signed-apt-hosting) |
| Godot | Extract the add-on ZIP at your project root | [Godot](../integrations/godot/README.md) |
| Unity | Package Manager → **Add package from tarball**, using the Unity `.tgz` | [Unity UPM](../integrations/unity/README.md) |
| Unreal | Extract the plugin ZIP into `Plugins`, then rebuild your C++ project | [Unreal](../integrations/unreal/README.md) |

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
[Windows](../docs/getting-started/windows.md) · [Linux](../docs/getting-started/linux.md).

Start with [bootstrap](getting-started/bootstrap.md), then [examples](../examples/README.md).
English is the default. Each maintained guide has an English/French link at the top.
Identifiers, JSON fields, code and upstream legal notices remain untranslated.

| Subject | Guide |
| --- | --- |
| SDK, PyPI, npm, vcpkg, APT and engine packages | [Distribution](development/distribution.md) |
| Setup, action, feedback by technology | [Bootstrap](getting-started/bootstrap.md) |
| Every example's modules and lifecycle | [Example code](getting-started/examples.md) |
| Desktop installation and controls | [Windows](getting-started/windows.md), [Linux](getting-started/linux.md) |
| Draw and save movements | [Configurator](guides/configurator.md) |
| Background controller and profiles | [Controller](guides/controller.md) |
| Schema-v2 JSON reference | [Configuration](reference/configuration.md) |
| C++ recognition contracts | [Engine API](reference/cpp.md) |
| Python/C#/native ABI | [C API](reference/c-abi.md), [integration](integrations/overview.md) |
| React/Vue/Next and browser setup | [JavaScript](integrations/javascript.md) |
| Complete repository architecture and execution | [Architecture](architecture/overview.md), [pipeline](architecture/lifecycle.md) |
| Hand observations and signs | [Hands](guides/hands.md) |
| Performance and measured limits | [Performance](architecture/performance.md) |
| Coding rules and checks | [Contributing](development/contributing.md) |
| Archives, pip/npm/Debian/editor packages | [Releasing](development/packaging.md) |
| Platform support and verification | [Release preparation](reference/support.md) |


[Changelog](development/CHANGELOG.md) · [Roadmap](development/ROADMAP.md)
