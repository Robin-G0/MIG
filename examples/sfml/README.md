# sfml camera example

[English](README.md) | [Français](README.fr.md)

Launch `mig-sfml` (`mig-sfml.exe` on Windows) without arguments.
The demo displays a mirrored camera, finger outlines and a prop for each wrist.
Follow the green rows upward into yellow to display Left/Right hand raised.
`mig-sfml-profile` starts empty and imports arbitrary configurator JSON via its
**Import profile** button. Actions appear in the window. Invalid imports preserve
the old profile; successful imports recalibrate. Both executables share one source.
Close the window to release the camera. No keys are injected.

Build with an installed MIG SDK:

```sh
cmake -S examples/sfml -B build/sfml -DCMAKE_PREFIX_PATH=/path/to/sdk
cmake --build build/sfml --config Release
```

If MIG is not installed, the build falls back to the full repository; bootstrap
native dependencies first. SFML 2.5+ development libraries are needed to build.
Linux builds also need Qt6 Widgets for the profile picker; Windows uses its native
file dialog. The archive includes the redistributable DejaVu font and its license.
Run `tools/build-examples.ps1` on Windows or `tools/build-examples.sh` on Linux
for tested builds. The examples archives contain binaries and sources side by side.
The hidden `--smoke` option uses generated frames without opening a camera.

`demo::Source` owns capture/inference; `demo::consume` dispatches library events;
`draw_frame` mirrors the camera and coordinate overlays, including wrist props.
The camera texture and pixel buffers are reused between frames. The simple C++
demo captures synchronously; move Source ownership to a worker for independent
rendering latency in your game. See [standalone instructions](../standalone.md).

[Complete source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).
