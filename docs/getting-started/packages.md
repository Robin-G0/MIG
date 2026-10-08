# Install release packages

[English](packages.md) | [Français](packages.fr.md)

Download packages for your operating system and architecture from
[GitHub Releases](https://github.com/Robin-G0/MIG/releases). Keep the complete
archive together after extraction. See the [support matrix](../reference/support.md)
for platform requirements and integration maturity.

## Desktop applications and examples

- `*-native` archives contain the configurator, controller, camera runtime and models.
  Follow the [configurator](../guides/configurator.md) or [controller](../guides/controller.md) guide.
- `*-examples` archives contain runnable demos and their sources. Follow the
  [standalone examples guide](../../examples/standalone.md).
- `*-standalone` archives contain one native example and its required dependencies.
  Start with the README at the archive root; keep its sibling folders together.
- JavaScript examples include local browser assets and Node launchers. Run
  `run.cmd` or `sh run.sh` in the chosen example folder and open the displayed URL.

## C++ SDK and vcpkg

Extract the matching `*-sdk` archive and pass its SDK directory to CMake:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/absolute/path/to/sdk
```

Link `MIG::core` and, for profile files, `MIG::format`. The SDK provides recognition
for supplied landmarks; camera-enabled applications use a separate native runtime.
See the [C++ installation guide](cpp.md).

For vcpkg, extract the release overlay and install with:

```sh
vcpkg install motion-input-grid --overlay-ports=/path/to/motion-input-grid-vcpkg-overlay
```

The overlay downloads the matching release source. See the
[vcpkg guide](../../ports/motion-input-grid/README.md) for supported triplets.

## Python and JavaScript

```sh
python -m pip install motion-input-grid
npm install motion-input-grid
npx mig-copy-assets public/mig
```

For offline installation, use the downloaded wheel or npm tarball:

```sh
python -m pip install /path/to/motion_input_grid-1.0.2-<tags>.whl
npm install /path/to/motion-input-grid-1.0.2.tgz
```

Python imports `mig`; the wheel provides recognition for supplied positions.
The npm package includes WebAssembly, browser models and framework adapters.
These packages do not install the desktop applications. See the
[Python](../../bindings/python/README.md) and
[JavaScript](../../bindings/javascript/README.md) guides.

## Debian/Ubuntu

Install a downloaded package matching your architecture:

```sh
sudo apt install ./motion-input-grid_1.0.2_amd64.deb
```

Use the `arm64` package on ARM64. The Debian package supplies the C++ SDK and
C ABI; the desktop applications are distributed separately. Installing by package
name requires an APT repository that provides MIG; downloading a `.deb` does not
configure one. See the [Linux guide](linux.md) for camera applications.

## Game engines

Use the package for your target platform and engine version:

- [Godot add-on](../../integrations/godot/README.md)
- [Unity UPM package](../../integrations/unity/README.md)
- [Unreal plugin](../../integrations/unreal/README.md)

These integrations are Preview and accept host-provided tracking. Their READMEs
explain installation, native dependencies and editor/export limitations.
