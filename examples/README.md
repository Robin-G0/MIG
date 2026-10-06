# Integration examples

[English](README.md) | [Français](README.fr.md)

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

Start with [standalone launch instructions](standalone.md) or the
[bootstrap guide](../docs/getting-started/bootstrap.md). Each visual integration has a raised-wrist
demo and an empty profile importer. The sample is [common/raised-hands.json](common/raised-hands.json).
Logical events give feedback; examples do not inject keyboard output.

| Integration | Guide |
| --- | --- |
| Tk/Python | [python-tkinter](python-tkinter/README.md) |
| Pygame | [pygame](pygame/README.md) |
| SDL2 / SFML | [SDL2](sdl2/README.md), [SFML](sfml/README.md) |
| Plain HTML / React / Vue / Next.js | [Web](web/README.md), [React](react/README.md), [Vue](vue/README.md), [Next](next/README.md) |
| Unity / Godot GDScript / C# / Unreal | [Unity](unity/README.md), [Godot](godot/README.md), [Unreal](unreal/README.md) |
| C++ positions / native RGB | [SDK](sdk-consumer/README.md), [native](native-consumer/README.md) |
| Shared helpers | [Common](common/README.md), [source walkthrough](../docs/getting-started/examples.md) |

Installed-library imports and checkout fallback are described per integration.
Native x64 and JavaScript archives include sources and documentation with their
built output. Editors and unsupported native ARM64 camera builds remain explicit
limits, not standalone binaries. See [release preparation](../docs/reference/support.md).
