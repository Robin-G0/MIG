# Controller

[English](controller.md) | [Français](controller.fr.md)

The controller runs profiles created in the configurator while you use another
application or play a game. It provides profile selection, binding feedback and
optional camera verification. Use the configurator to draw or change movements.

## First run

1. Close the configurator's camera, then open `mig-controller`.
2. Click **Import profile** and choose a JSON file saved by the configurator.
3. Give the profile a recognizable name. On Windows, edit the name field and click
   **Save name**. On Linux, click **Rename profile**.
4. Click **Start**, face the camera and keep your shoulders visible for calibration.
5. Try a movement: its binding flashes green for 900 ms. Hold and Repeat bindings
   remain highlighted while their terminal conditions are satisfied.
6. Enable **Keyboard output** when ready, then focus your game or target application.

Keyboard output starts disabled every time you launch the controller. Highlighting
works with output disabled. Windows uses SendInput; Linux requires X11/XTest and
keys available in the current keyboard layout. Native Wayland output is unavailable.
Stop, lost tracking, expired observations and closing the controller release held keys.

## Profiles

The profile dropdown switches between imported profiles, including while tracking
is running. Switching releases keys and clears recognition from the previous
profile. Windows keeps capture running; Linux restarts its owning capture worker.
The new profile calibrates before producing actions. The last selected profile
and its name are restored on the next launch; camera and keyboard output still
require explicit activation.

Imported files are copied into your application-data folder. Moving the original
file does not break an imported profile. **Export file** saves the selected profile
as a normal JSON file that you can open in the configurator or use with the libraries.
To use an edited configuration, import that file again as a new profile.
Invalid imports preserve the running profile. Up to 64 profiles can be stored.

Profile data is stored here:

- Windows: `%LOCALAPPDATA%\MIG\controller`.
- Linux: `$XDG_DATA_HOME/mig/controller`, or
  `~/.local/share/mig/controller` when `XDG_DATA_HOME` is unset.

`profiles.json` stores names and the last selection; numbered JSON files contain
standard schema-v2 configurations. Back up the whole folder to keep your profiles.
This list is separate from the [configuration format](../reference/configuration.md).

## Views

The default view displays bindings without a camera preview. This keeps feedback
available while avoiding preview conversion and rendering work.

In **View**, enable **Camera preview** to display both bindings and the mirrored
camera image. **Grid**, **Hand detections** and **Body dots** toggle overlays
independently. Hiding a hand overlay does not disable hand recognition required by
the profile. Light and dark themes are available through **Dark mode**.
Scrollbars in lists, dropdowns and log panes follow the selected theme on both
Windows and Linux, in the controller and configurator.

**Compact background window** reduces the controller to a small window with an
**Open** button. Tracking and enabled keyboard output continue. **Open** restores
the previous window. Normal taskbar minimization also keeps tracking active.
Preview copying/conversion and painting are skipped while the preview is hidden.
The pose model still processes camera frames; hiding the UI does not reduce the
recognition quality or disable required hands.

## Verify a binding

Enable **View → Verify bindings**, then select a binding in the list. Its colored
regions appear over the camera: green Required, red Forbidden, yellow Trigger and
purple Interaction. White outlines indicate validated, triggered or currently held
regions. Invalid finger/sign conditions or missing landmarks use red feedback.
Order numbers appear in the regions; mirroring follows the recognized candidate.

Verification pauses all keyboard output while keeping recognition feedback live.
Turn verification off before playing. You can recalibrate without modifying the
profile. Verification does not edit regions or isolate recognition to one input;
all bindings continue to report their accepted actions.

## Build and troubleshooting

See [Windows applications](../getting-started/windows.md) or [Linux applications](../getting-started/linux.md)
for dependencies and build commands. Keep the runtime libraries and models beside
the packaged executable. A profile with hand tracking requires a hands-enabled build.
If a saved profile is missing or invalid, the controller reports the problem;
import a valid file or select another saved profile.

The controller uses the same recognition engine and key scheduler as the
configurator. Automated tests cover profile persistence, view changes, verification
and cancellation. Camera accuracy and delivery to a particular game require a
check on your desktop.
