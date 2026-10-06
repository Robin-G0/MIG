# Bootstrap a project

[English](bootstrap.md) | [Français](bootstrap.fr.md)

Choose a complete examples archive for your platform when you want immediate
camera feedback. Extract the whole archive; its shared `runtime`, `bindings`,
`examples/common` and license folders are required. Source files sit beside
executables or compiled pages. [Standalone instructions](../../examples/standalone.md)
list runtime and operating-system requirements.

## Configure one action

Use [raised-hands.json](../../examples/common/raised-hands.json) as your first profile.
It defines `left_raise` and `right_raise`: anatomical wrist 15/16 travels through
four green Required rows, then a yellow Trigger row. Each row spans the grid.
Keep shoulders visible for about one second, lower your hands, then raise either
one. The view mirrors once; left/right feedback refers to your anatomy.
Copy the profile before editing it. Change `action` to your application's command
name; preserve unique input IDs and the [schema-v2 rules](../reference/configuration.md).

The demo variant loads that profile. The profile variant starts empty: click
**Import profile**, choose JSON and recalibrate. Invalid imports preserve the
previous configuration. Accepted events appear on screen and in the terminal;
examples do not type keys into another application.

## Run and adapt each integration

| Technology | Start from an extracted archive | Connect feedback to your app |
| --- | --- | --- |
| Python/Tk | `examples/python-tkinter/main.exe` on Windows; `./examples/python-tkinter/main` on Linux | Replace `announce` or handle events returned by `InputSource.take()` |
| Pygame | Same launch names in `examples/pygame` | Replace `announce(events, ...)` in the loop |
| SDL2 | `examples/sdl2/mig-sdl2.exe` or `./examples/sdl2/mig-sdl2` | Handle each event in `demo::consume` |
| SFML | Corresponding `mig-sfml` executable | Same shared consume function |
| Plain browser | `run.cmd` or `sh run.sh` in `examples/web` | Set `MIGSession`'s `onAction` |
| React | Launcher in `examples/react`; `/` or `/profile.html` | `useMIG({onAction})` |
| Vue | Launcher in `examples/vue`; `/` or `/profile.html` | Composable `useMIG({onAction})` |
| Next.js | Launcher in `examples/next`; `/` or `/profile` | Client component's React `onAction` |
| C++ supplied positions | Build `examples/sdk-consumer`, then run `mig-sdk-example configs/default.json` | Read the span returned by `engine.update(frame)` as demonstrated by the consumer |
| Native C++ estimator | Build `examples/native-consumer`; see its README for arguments | Supply RGB, update the same engine, consume events |
| Unity | Install runtime UPM, then copy demo scripts/SyntheticFrames and sample Resource | `OnAction` UnityEvent or `SubmitFrame(packet)` |
| Godot C# | Attach `csharp/MigRaisedHands` to a Node3D, copy bridge/profile/native lib | Connect `MotionAction`; call `SubmitFrame` from your provider |
| Godot GDScript | Build `gdscript/native`, open the generated project and choose a scene | Connect `motion_action`; call `submit_frame` from your provider |
| Unreal | Place `MigExample` under `Plugins`, configure its native SDK | Subscribe to `OnMotion`; call `SubmitFrame` |

GUI profile variants use `profile.exe`/`profile` for Python and `mig-*-profile`
for C++. Editor components have both a raised-hands and a generic variant.
Game engines require their installed editor and a camera/landmark provider;
`UseSyntheticDemo` checks wiring without a camera. They are not turnkey exports.

## Use installed libraries or the checkout

Python sources require Python 3.10+, Pillow and Pygame (Tk for the picker):

```sh
python -m pip install pillow pygame
python examples/python-tkinter/main.py
python examples/pygame/profile.py
```

Scripts prefer an installed `mig` import, then use `bindings/python` from a full
checkout. That package still needs the matching native C ABI library. Set
`MIG_LIBRARY` and `MIG_RUNTIME` only when overriding discovery. Frozen viewers
include Python/UI dependencies and use the archive's native runtime.
See the [Python package](../../bindings/python/README.md).

C++ graphics consumers first try `find_package(MIG CONFIG)` and otherwise build the checkout.
From a configured native build, install and configure an example:

```sh
cmake --install build/linux-native --prefix "$PWD/install"
cmake -S examples -B build/graphics -DCMAKE_PREFIX_PATH="$PWD/install"
cmake --build build/graphics --parallel 2
```

On Windows, add `--config Release` to build/install. SDL2/SFML development
packages and the platform toolchain are needed for rebuilding, not for bundled
execution. [Integration](../integrations/overview.md) explains RGB and supplied-pose hosts.

JavaScript workspaces use the same package name as consumers. Prepare WASM and
checksummed assets, then build the frameworks:

```sh
cmake -P tools/bootstrap-web.cmake
node tools/bootstrap-browser.mjs
npm ci
npm run prepare:javascript
npm run build:examples
```

Build `build/web/web/mig.mjs` and `mig.wasm` with Emscripten first; the
[JavaScript guide](../integrations/javascript.md) gives the full commands and adapter snippets.
The npm package contains local models and MediaPipe/WASM assets; run
`mig-copy-assets public/mig` in a consumer. Serve over localhost or HTTPS.
Only an explicit Start requests camera permission. React cleanup survives
StrictMode; Next initializes browser resources only on the client.

For C# use [MigTracker](../../bindings/dotnet/README.md); for lower-level Python/C#
contracts read the [C API](../reference/c-abi.md). Handle logical events instead of forwarding
keyboard injection from examples. Dispose trackers and stop capture on exit.

## Choose a CMake preset

A preset stores the build options and output directory. VS Code's CMake Tools
uses these same presets; choosing one configures the project, it does not run it.

`CMakePresets.json` is shared in Git. Keep machine-specific overrides in
`CMakeUserPresets.json`, which is ignored, including local toolchain paths.

- `release`: optimized configurator, controller and engine. Bootstrap the native
  dependencies first; see [Windows](windows.md) or [Linux](linux.md).
- `debug`: the same applications with debugging information.
- `sdk-release` / `sdk-debug`: portable engine and bindings only, without camera
  applications or MediaPipe. A C++20 toolchain is required; the first configuration
  fetches the JSON dependency when it is not already installed.

```sh
cmake --preset sdk-release
cmake --build --preset sdk-release
ctest --preset sdk-release
```

For the desktop applications, select `release` after bootstrap. A fresh clone
contains no downloaded dependencies or binaries; a missing-bootstrap error is
resolved by the platform bootstrap command, not by changing optimization mode.

## Verify your changes

Use each viewer's hidden `--smoke` for generated positions. It checks event/render
wiring, not your camera. Keep the real camera, sign accuracy, held-output release
and target-OS checks on your own release checklist. Read the
[source walkthrough](examples.md) before modifying ownership or mirroring.
