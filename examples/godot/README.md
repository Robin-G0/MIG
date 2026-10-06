# Godot 4 desktop examples

[English](README.md) | [Français](README.fr.md)

Choose the folder matching your project:

- [GDScript](gdscript/README.md): standard editor, native GDExtension bridge,
  ready-to-open raised-hands and profile-import scenes.
- [C#](csharp/README.md): Godot .NET editor, shared .NET bridge and Node3D components.

Both use MIG's C++ recognition engine through the same C ABI. They show action
feedback, emit game signals and accept observations from your pose provider.
Synthetic mode demonstrates wrist raising without a camera. Failed profile
imports preserve the current configuration.

Windows and Linux desktop only. Build native libraries for your editor/export
architecture. These examples supply no estimator or web export.

[Source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).

[Separate runtime package](../../integrations/godot/README.md).
