# Python/Tkinter camera examples

[English](README.md) | [Français](README.fr.md)

Run `python3 examples/python-tkinter/main.py` for the raised-hands demo, or
`python3 examples/python-tkinter/profile.py` to import a configurator JSON profile.
Both start without arguments, display a mirrored camera and wrist-following props,
and show all accepted actions on screen. The importer has an **Import profile** button.
Invalid profiles keep the previous configuration running; successful imports recalibrate.

Install Python 3.10+, Tk and Pillow. On Ubuntu, install `python3-tk` and
`python3-pil.imagetk`. See [standalone setup](../standalone.md) for runtime discovery.
Installed `mig` is preferred, with the complete repository as fallback.
The inference worker owns the camera and tracker, including imports and teardown.
Closing the window joins the worker before destroying UI resources.

[Complete source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).
