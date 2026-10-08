# Integration examples

[English](README.md) | [Français](README.fr.md)

See Motion Input Grid (MIG) recognize a raised hand, then import a profile of your
own. The camera demos show a mirrored preview, wrist-following props and feedback
identifying the accepted action. They report events without sending keyboard keys.

## Launch a demo

> [!NOTE]
> On Debian, the configurator, controller and some native examples are still
> being developed and tested. They may not yet work fully.

Download one **`*-standalone`** tutorial or the combined **`*-examples`** archive from
[Releases](https://github.com/Robin-G0/MIG/releases) and extract it completely.
Individual archives launch from their root; the table below gives combined-archive
paths. Native x64 archives bundle runtime libraries/models. Individual browser
archives require Node 22.12+; the combined JavaScript archive bundles Node.
Both include compiled pages, source and local browser assets.

1. Start one viewer from the table below and allow camera access.
2. Keep both shoulders visible for a second to calibrate, then lower your hands.
3. Raise either wrist through the green region into the yellow row.
4. Open the profile variant to import a JSON file saved by the configurator.

The flying props and hand bones follow your wrists. The image is mirrored once;
feedback is anatomical.

> [!TIP]
> Keep the extracted folders together. Run one camera viewer at a time.
> Lower your hand before repeating the movement.

| Platform | Raised-hands viewer | Import your profile |
| --- | --- | --- |
| Windows x64 Tk/Pygame | `examples/python-tkinter/main.exe`, `examples/pygame/main.exe` | Corresponding `profile.exe` |
| Linux x64 Tk/Pygame | `./examples/python-tkinter/main`, `./examples/pygame/main` | Corresponding `./profile` |
| Windows x64 SDL2/SFML | `examples/sdl2/mig-sdl2.exe`, `examples/sfml/mig-sfml.exe` | `mig-sdl2-profile.exe`, `mig-sfml-profile.exe` |
| Linux x64 SDL2/SFML | `./examples/sdl2/mig-sdl2`, `./examples/sfml/mig-sfml` | Corresponding `-profile` launcher |
| JavaScript archive | `run.cmd` on Windows x64, `sh run.sh` on Linux x64/ARM64 inside web/react/vue/next | `/profile.html` (web/React/Vue), `/profile` (Next) |

### Native prerequisites

Windows native viewers still require the Microsoft Visual C++ 2022 Redistributable.
That prerequisite prevents a claim that every desktop archive runs on a completely
unprepared Windows machine. Linux x64 targets Ubuntu 22.04+, Debian 12+, recent
Fedora/Arch with a graphical X11/XWayland session; glibc, drivers and the display
server are system dependencies. SDK ARM64 works with supplied positions;
the pinned upstream native camera runtime has no Linux ARM64 build. Windows ARM64
native executables have not been built.

### Browser launchers

The JavaScript archive includes portable Node runtimes, compiled pages, sources,
models, MediaPipe and WASM. The launchers serve localhost:8820. Open that URL and
click Start; a modern browser with camera support is required. No npm install,
Node installation or CDN download is needed for execution. Stop the server before
starting another viewer. Plain web files must also be served, never opened via file://.

### Import a profile

Profile viewers start empty. Click Import profile and select schema-v2 JSON;
invalid imports preserve the current configuration. SDL2/Pygame also support
dropping the file. Accepted actions/IDs appear without keyboard injection.
Hand bones only appear when the profile requests hand inference.

## Choose an integration

| Host | Example | Prerequisites when running from source |
| --- | --- | --- |
| Python GUI | [Tkinter](python-tkinter/README.md), [Pygame](pygame/README.md) | Python, UI dependencies and native camera runtime |
| C++ GUI | [SDL2](sdl2/README.md), [SFML](sfml/README.md) | C++ toolchain, graphics dependencies and native SDK |
| Browser | [Plain HTML](web/README.md) | Local web server and prepared WASM/assets |
| Framework | [React](react/README.md), [Vue](vue/README.md), [Next.js](next/README.md) | Node.js and npm package/assets |
| Game engine | [Godot](godot/README.md), [Unity](unity/README.md), [Unreal](unreal/README.md) | Editor/toolchain and a landmark provider |
| Console / custom host | [C++ positions](sdk-consumer/README.md), [Native RGB](native-consumer/README.md) | SDK; camera estimator only for the RGB sample |

Each visual integration offers a raised-wrist demo and an initially empty profile
importer. They share [raised-hands.json](common/raised-hands.json) and the C++ engine.
Game engine samples require their editor and include synthetic provider trials; they are not
standalone camera exports. SDK consumers are console contract samples rather
than camera UIs. See the [support matrix](../docs/reference/support.md).

## Adapt the code

Install **`motion-input-grid`** through [pip](../bindings/python/README.md),
[npm](../bindings/javascript/README.md), or use the [C++ SDK / CMake guide](../docs/getting-started/cpp.md).
[Other packages](../docs/getting-started/packages.md) cover vcpkg, Debian and editor integrations.
Per-example guides explain installed-library use and full-checkout fallback.

[Bootstrap](../docs/getting-started/bootstrap.md) connects a profile to action
feedback. The [source walkthrough](../docs/getting-started/examples.md) explains
the modules and lifecycle. Python/C++ folders expose `example_usage.py`/`.hpp`;
framework folders expose `example_usage.mjs`, and editor guides name their
component lifecycle. Local `support/` files cover camera
discovery, coordinates and drawing. Start by replacing the action callback with
your application's command.

## Sources and runtimes

Frozen Python viewers include the interpreter, Tk/Pillow/Pygame and native
recognition assets. No Python installation or pip command is needed. Source
scripts remain beside them for modification. Scripts run with Python 3.10+ and
`python -m pip install motion-input-grid pillow pygame`; Tk is also required for file pickers.
They prefer an installed `mig`, then fall back to the archive's binding.

Checkout examples also find the native runtime in `build/release/bin` or
`build/debug/bin` when built with CMake presets. Automatic selection requires
both the platform's MediaPipe library and the pose model; model files alone do
not make a runtime usable. Set `MIG_RUNTIME` to choose another runtime directory.

### Runtime overrides and diagnostics

`MIG_LIBRARY`/`MIG_RUNTIME` optionally override native discovery. Hidden `--smoke`
runs generated observations without camera permission. Closing joins inference
before toolkit shutdown. Upstream MediaPipe can log internal telemetry warnings;
those alone do not prove a crash, and offline execution does not promise upstream
telemetry silence. Read [release preparation](../docs/reference/support.md)
for platform support and verification results.

## Archive layout and rebuilding

Each individual tutorial is its own archive root with a README, entry point,
integration file, configuration, dependencies and licenses. Copy that complete
folder. The guides separately document source-only prerequisites and commands
for rebuilding outside this repository. Combined archives keep larger runtimes
shared inside the archive. They are useful to compare viewers; individual
archives are the portable unit for one tutorial.
