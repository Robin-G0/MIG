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
