# Configurator

[English](configurator.md) | [Français](configurator.fr.md)

The Motion Input Grid (MIG) configurator turns drawn body movements into JSON
profiles. Choose a tracked body part, draw its route and set the action to emit.
Run the saved profile in the [controller](controller.md) or an application using MIG.

## Find and launch the application

In [Releases](https://github.com/Robin-G0/MIG/releases), choose a matching native
archive when available:

| System | Archive | File after extraction |
| --- | --- | --- |
| Windows x64 | `motion-input-grid-<version>-windows-x64-native.zip` | `windows/mig-configurator.exe` |
| Linux x64 | `motion-input-grid-<version>-linux-x64-native.tar.gz` | `./mig-configurator` |

Paths start at the extracted archive's main folder. On Windows, open the executable.
On Linux, open a terminal in that folder and run `./mig-configurator`; this launcher
sets up the bundled libraries before starting the binary in `bin/`. Keep the
archive's DLLs, models, configurations and other resources together. Windows
requires the Visual C++ 2022 x64 runtime; Linux archives require glibc 2.35+.

After building from source:

| Method | File relative to the repository root |
| --- | --- |
| Windows `release` preset | `build/release/bin/mig-configurator.exe` |
| Default `tools/build-windows.ps1` script | `build/windows/bin/mig-configurator.exe` |
| Linux guide's build | `build/linux-apps/bin/mig-configurator` |

Follow the [Windows](../getting-started/windows.md) or
[Linux](../getting-started/linux.md) guide to build. The Linux guide also supplies
`--runtime` to locate models and the MediaPipe library. The `sdk-*` presets and
library packages do not build these applications.

## Create your first profile

1. Open the configurator and start the camera. Keep both shoulders visible during calibration.
2. Add an input. On Windows, **Add/Edit** opens its drawing editor; Linux uses the sidebar.
3. Choose the tracked body part, such as a wrist, and draw its regions on the grid.
4. Set the input name, action and optional keyboard binding.
5. Save the profile as JSON. On Windows, apply the input to the document before **File → Save**.
6. Stop the camera, open the controller and import the saved profile.

Region colors describe their conditions:

| Color | Condition |
| --- | --- |
| Green — Required | Visit this region; numbered regions specify the route's order. |
| Red — Forbidden | Avoid this region. |
| Yellow — Trigger | Complete the movement here to emit its action. |
| Purple — Interaction | Perform the selected hand sign in this region, optionally holding it. |

On Windows, use **Save layer** after changing a layer's body part; **Apply** saves
that input into the open document, and **File → Save** writes the JSON file.
Pro mode exposes hand signs, finger rules and advanced constraints. Linux provides
advanced fields through its Pro JSON editor; its drawing tools differ from Windows.
The [Windows controls](../getting-started/windows.md#workspace-and-drawing) and
[Linux controls](../getting-started/linux.md) describe the available tools.

## Check and use the profile

Keep your shoulders visible so the grid can follow your body. Try the movement
and watch for action feedback. Use the [hand tracking guide](hands.md) for signs
and finger conditions. Close one application's camera before opening the other's.

Keyboard output starts disabled. The controller guide explains how to enable it
and use Single press, Hold and Repeat. **View** controls the theme, preview overlays
and logs. The [configuration reference](../reference/configuration.md) describes
all saved fields and action modes.
