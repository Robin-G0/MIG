# Integration examples

[English](README.md) | [Français](README.fr.md)

See Motion Input Grid (MIG) recognize a raised hand, then import a profile of your
own. The camera demos show a mirrored preview, wrist-following props and feedback
identifying the accepted action. They report events without sending keyboard keys.

## Try a demo

> [!NOTE]
> On Debian, the configurator, controller and some native examples are still
> being developed and tested. They may not yet work fully.

Download a **`*-examples`** archive from
[Releases](https://github.com/Robin-G0/MIG/releases), extract it completely and
follow the [standalone launch instructions](standalone.md). Native x64 and browser
archives include their runtimes and sources; no build is needed to try them.

1. Start one viewer and allow camera access.
2. Keep both shoulders visible for calibration, then lower your hands.
3. Raise either wrist through the green rows into the yellow row.
4. Open the profile variant to import a JSON file saved by the configurator.

> [!TIP]
> Keep the extracted folders together. Run one camera viewer at a time.
> Lower your hand before repeating the movement.

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
Game engine samples require their editor and observation provider; they are not
standalone camera exports. See the [support matrix](../docs/reference/support.md).

## Adapt the code

Install **`motion-input-grid`** through [pip](../bindings/python/README.md),
[npm](../bindings/javascript/README.md), or use the [C++ SDK / CMake guide](../docs/getting-started/cpp.md).
[Other packages](../docs/development/distribution.md) cover vcpkg, Debian and editor integrations.
Per-example guides explain installed-library use and full-checkout fallback.

[Bootstrap](../docs/getting-started/bootstrap.md) connects a profile to action
feedback. The [source walkthrough](../docs/getting-started/examples.md) explains
the modules and lifecycle; [shared helpers](common/README.md) covers camera
discovery, coordinates and drawing. Start by replacing the action callback with
your application's command.
