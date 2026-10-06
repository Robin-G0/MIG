# Preparing the first release

[English](packaging.md) | [Français](packaging.fr.md)

## Runnable examples archives

`build/examples/windows-x64` and `build/examples/linux-x64` contain camera demos,
their sources and matching native runtime/models. Executables are beside `main.cpp`
in `examples/sdl2` and `examples/sfml`; Frozen Python viewers and their scripts are included as well. Browser/framework
examples have a separate archive. Launch desktop examples without arguments.

Build Windows examples with `tools/build-examples.ps1` after building/installing
the native SDK, then freeze the Python viewers before packaging (commands below).
The script verifies pinned official SDL2/SDL2_ttf/SFML development archives before extraction.
Linux release preparation calls `tools/build-examples.sh`, which builds, tests and
packages both variants of each C++ demo. For packaging only, use
`python3 tools/package-examples.py --platform linux-x64`.

Outputs are separate `mig-1.0.0-*-examples` archives in `build/releases`, with
file manifests and SHA256 sidecars. See [standalone instructions](../../examples/standalone.md)
for bundled execution, source dependencies, installed-library fallback and limits.

Build, test and inspect the 1.0.0 artifacts before uploading them. These scripts do not publish to GitHub, PyPI or an apt repository.

## Supported release variants

| Archive/package | Contents | Requirements |
| --- | --- | --- |
| Windows x64 native ZIP | Controller, configurator, SDK, MediaPipe/models | Windows 10/11 x64; VC++ 2022 runtime |
| Windows ARM64 native ZIP | Same contents, when built on an ARM64 toolchain | Windows ARM64; ARM64 VC++ runtime; hardware verification required |
| Linux x64 native TAR.GZ | Qt applications, SDK, bundled shared dependencies/models | glibc 2.35+, X11/XWayland, host graphics drivers |
| Linux ARM64 SDK TAR.GZ | Positions SDK, hands algorithms, C ABI, examples | glibc 2.35+, libstdc++ from GCC 11+; host supplies landmarks |
| `libmig-dev` amd64/arm64 DEB | Positions SDK headers/libraries and CMake exports | Ubuntu 22.04+/Debian 12+, C++20 compiler, CMake 3.25+ for consumers |
| Python wheel/sdist | ctypes plus bundled C ABI; canonical source archive | Python 3.10+; matching platform wheel, or C++20/CMake for source builds |

The pinned [MediaPipe 0.10.35 files](https://pypi.org/project/mediapipe/0.10.35/#files)
provide Windows ARM64, Windows x64 and Linux x64 binaries. They provide no Linux
ARM64 runtime. Linux ARM64 camera inference and applications are therefore absent,
rather than shipping an x64 runtime under an ARM64 label.

Linux packages target Ubuntu 22.04, Debian 12 and newer glibc distributions,
including Fedora and Arch. Alpine/musl is unsupported. Native Wayland keyboard
injection is unsupported; Linux uses XTest on X11. Distribution startup smoke
tests do not verify a physical desktop, camera or keyboard output.

## Build and package Windows

```powershell
./tools/build-windows.ps1
./tools/package-distribution.ps1 -CMakePath cmake
./tools/package-windows-release.ps1
```

For ARM64, install the Visual Studio ARM64 C++ tools and matching Windows SDK,
use a separate build directory, then package that directory explicitly:

```powershell
./tools/build-windows.ps1 -Architecture ARM64 -BuildDirectory build/windows-arm64
./tools/package-distribution.ps1 -WindowsBuild build/windows-arm64 -Destination distribution-arm64
./tools/package-windows-release.ps1 -Distribution distribution-arm64/windows -Architecture arm64
```

Cross-compilation skips running ARM64 tests on x64. Run CTest and the applications
on Windows ARM64 before publication. Install the matching Microsoft Visual C++
Redistributable; the archive does not copy system DLLs from the build machine.

## Build and package Linux

Run these commands in the checkout on a Docker host:

```sh
docker build -f tools/linux-release.Dockerfile -t mig-linux-release:22.04 tools
docker run --rm -v "$PWD:/src" -w /src mig-linux-release:22.04 \
    bash tools/build-linux-release.sh
```

The script builds/tests x64 applications and the ARM64 SDK, installs both SDKs,
builds installed consumers, creates local Debian packages and TAR.GZ archives,
then builds/checks Python distributions. ARM64 deterministic tests run under QEMU;
this verifies behavior but does not establish ARM hardware performance.

Extract the native TAR.GZ and use its root `mig-controller` or `mig-configurator`
launcher. Launchers set library/plugin paths relative to the extracted archive.
Move the entire directory together. SDKs retain relocatable CMake exports and
symbolic links. Do not flatten their `lib` directories.

The Qt libraries remain dynamically replaceable. Archives include package
copyright notices and license texts for bundled dependencies, plus a dependency
inventory and file hashes in `manifest.json`. Qt distribution obligations and
corresponding source availability need your release review; package notices alone
do not constitute that verification. Sources of the Ubuntu dependency packages
can be obtained with `apt-get source <package>` on the matching Ubuntu release
after enabling `deb-src`. See [Qt deployment](https://doc.qt.io/qt-6/linux-deployment.html)
and [Qt LGPL guidance](https://doc.qt.io/qt-6/lgpl.html).

## Manual publication

Verify archive SHA256 sidecars and inspect contents. Test camera start/stop,
hand accuracy, mirroring and held/repeated keyboard release on real hardware.
Review the [support and validation limits](../reference/support.md) and remaining limits.

For npm, first follow [JavaScript preparation](../integrations/javascript.md), then run
`npm pack --workspace @mig-input/browser --pack-destination build/releases`.
Inspect the tarball, install it into a separate application, copy assets with
`npx mig-copy-assets public/mig`, and test your camera before uploading manually.
Publishing `@mig-input/browser` requires access to the `@mig-input` npm scope. React/Vue are optional peers; Next.js uses React. The package includes
WASM and model files. `npm run package:examples` creates the separate portable
JavaScript examples archive, SHA256 sidecar and file manifest.

For PyPI, use `tools/package-python.py` to build a platform wheel containing the
native C ABI and a self-contained source archive. Repair Linux platform tags with
auditwheel, install/test the generated wheel in isolation, and run
`python -m twine check` before manual upload. Do not publish old `none-any` wheels.
See [ecosystem distribution](distribution.md) for SDK, PyPI, npm, vcpkg, signed
APT hosting and runtime Godot/Unity/Unreal packages.

For Debian/Ubuntu, test local installation with `sudo apt install ./libmig-dev_*.deb`.
A `.deb` can be installed locally; `apt install libmig-dev` by name requires your
own signed apt repository or distribution acceptance. Repository signing, hosting,
distribution submission and uploads remain manual. The development package includes
the ABI-1 shared library and static C++ libraries; no GUI or estimator is installed.

Set `-DMIG_PACKAGE_MAINTAINER="Your Name <your-email>"` when configuring the Debian
build for publication; the default is `Robin-G0 <robin.g0.dev@gmail.com>`.
The manual `Prepare release candidates` GitHub workflow performs build/package
checks and uploads workflow artifacts only. It creates no public release or registry
upload. Run it yourself after pushing, then inspect its artifacts and logs.

## Frozen viewers and editor source bundles

The target host needs PyInstaller 6.16.0, Pillow and Pygame to build frozen viewers.
Windows build preparation uses Python 3.11 with Pillow 11.3.0/Pygame 2.6.1 here:

```powershell
python -m pip install pyinstaller==6.16.0 pillow==11.3.0 pygame==2.6.1
python tools/freeze-python-examples.py
python tools/package-examples.py --platform windows-x64
```

Linux's release Dockerfile includes libpython3.10, UI dependencies and PyInstaller;
build-examples.sh freezes before packaging. Freeze on the target OS, not by relabeling
an executable. Native example archives include SDKs for source rebuilding;
use CMAKE_PREFIX_PATH pointing to sdk. Python executables include dependencies,
while sources use installed mig first and then the archive's binding.

For the JavaScript archive, prepare checksummed local browser and Node assets:

```sh
node tools/bootstrap-browser.mjs
node tools/bootstrap-node.mjs
npm run build:examples
npm run package:examples
```

It includes Windows x64 and Linux x64/ARM64 Node, with run.cmd/run.sh in each
viewer folder, and local MediaPipe/WASM/models. A modern browser remains required;
no Node installation, npm install or CDN download is needed for built execution.

Prepare separate editor source/SDK archives with:

```sh
python tools/package-editor-examples.py --sdk build/examples-sdk/windows --platform windows-x64
```

Use the matching Linux installed SDK/platform for Linux source bundles. These
are editor-import candidates, not compiled games. Unity includes package metadata;
the tool adds C# bridge sources. Unreal retains its .uplugin and SDK staging layout.
Godot includes separate C# components and GDScript scenes with GDExtension sources;
build the latter for the target editor/export. No Asset Library submission is performed.
Editor imports, exports, native dependencies, package names, marketplace metadata
and signatures need manual verification. See the [current checklist](../reference/support.md).

Building the JavaScript archive requires Python 3 as well as Node. The Node
bootstrap extracts Windows ZIPs with Python's zipfile on either build host and
Linux tar.xz archives with tar, after verifying their pinned SHA256 hashes.
Only the Node executable and LICENSE are extracted for each platform.
The TAR writer preserves Linux executable permissions when packaging on Windows.
Python is not
required to run the generated browser examples.
