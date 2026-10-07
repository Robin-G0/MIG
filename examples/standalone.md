# Run the standalone examples

[English](standalone.md) | [Français](standalone.fr.md)

Download the **`*-examples`** archive matching your system from
[Releases](https://github.com/Robin-G0/MIG/releases). It includes built applications,
their sources and dependencies. Examples recognize a raised hand or run an imported
profile without sending keys to other applications.

## Launch a demo

> [!NOTE]
> On Debian, the configurator, controller and some native examples are still
> being developed and tested. They may not yet work fully.

Extract the complete platform archive. Run one camera example at a time.
Keep shoulders visible for a second, lower your hands, then raise either wrist
through the green rows into the yellow row. The flying props and hand bones
follow your wrists. The image is mirrored once; feedback is anatomical.

| Platform | Raised-hands viewer | Import your profile |
| --- | --- | --- |
| Windows x64 Tk/Pygame | `examples/python-tkinter/main.exe`, `examples/pygame/main.exe` | Corresponding `profile.exe` |
| Linux x64 Tk/Pygame | `./examples/python-tkinter/main`, `./examples/pygame/main` | Corresponding `./profile` |
| Windows x64 SDL2/SFML | `examples/sdl2/mig-sdl2.exe`, `examples/sfml/mig-sfml.exe` | `mig-sdl2-profile.exe`, `mig-sfml-profile.exe` |
| Linux x64 SDL2/SFML | `./examples/sdl2/mig-sdl2`, `./examples/sfml/mig-sfml` | Corresponding `-profile` launcher |
| JavaScript archive | `run.cmd` on Windows x64, `sh run.sh` on Linux x64/ARM64 inside web/react/vue/next | `/profile.html` (web/React/Vue), `/profile` (Next) |

Frozen Python viewers include the interpreter, Tk/Pillow/Pygame and native
recognition assets. No Python installation or pip command is needed. Source
scripts remain beside them for modification. Scripts run with Python 3.10+ and
`python -m pip install motion-input-grid pillow pygame`; Tk is also required for file pickers.
They prefer an installed `mig`, then fall back to the archive's binding.

Windows native viewers still require the Microsoft Visual C++ 2022 Redistributable.
That prerequisite prevents a claim that every desktop archive runs on a completely
unprepared Windows machine. Linux x64 targets Ubuntu 22.04+, Debian 12+, recent
Fedora/Arch with a graphical X11/XWayland session; glibc, drivers and the display
server are system dependencies. SDK ARM64 works with supplied positions;
the pinned upstream native camera runtime has no Linux ARM64 build. Windows ARM64
native executables have not been built.

The JavaScript archive includes portable Node runtimes, compiled pages, sources,
models, MediaPipe and WASM. The launchers serve localhost:8820. Open that URL and
click Start; a modern browser with camera support is required. No npm install,
Node installation or CDN download is needed for execution. Stop the server before
starting another viewer. Plain web files must also be served, never opened via file://.

Profile viewers start empty. Click Import profile and select schema-v2 JSON;
invalid imports preserve the current configuration. SDL2/Pygame also support
dropping the file. Accepted actions/IDs appear without keyboard injection.
Lower a raised hand and follow the path again to retrigger. Hand bones only
appear when the profile requests hand inference.

Unity, Godot and Unreal folders contain source components and setup docs. They
need their editor/toolchain and a pose provider; standalone game exports are
not supplied. SDK consumers are console contract samples rather than camera UIs.
The [bootstrap](../docs/getting-started/bootstrap.md) and [code walkthrough](../docs/getting-started/examples.md)
explain every integration and the shared helpers.

`MIG_LIBRARY`/`MIG_RUNTIME` optionally override native discovery. Hidden `--smoke`
runs generated observations without camera permission. Closing joins inference
before toolkit shutdown. Upstream MediaPipe can log internal telemetry warnings;
those alone do not prove a crash, and offline execution does not promise upstream
telemetry silence. Read [release preparation](../docs/reference/support.md)
for platform support and verification results.

## Sources and runtimes

Checkout examples also find the native runtime in `build/release/bin` or
`build/debug/bin` when built with CMake presets. Automatic selection requires
both the platform's MediaPipe library and the pose model; model files alone do
not make a runtime usable. Set `MIG_RUNTIME` to choose another runtime directory.
