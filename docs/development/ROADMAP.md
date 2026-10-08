# Roadmap

[English](ROADMAP.md) | [Français](ROADMAP.fr.md)

Priorities follow validation and user feedback; no delivery dates are promised.
Godot, Unity and Unreal stay **Preview** until editor and exported-game testing
covers enough supported versions/platforms. Current evidence is in the [support matrix](../reference/support.md).

Planned improvements focus on:

- Broader webcam compatibility across integrated and USB cameras, resolutions
  and frame rates, with reliable recovery from tracking loss.
- Easier installation through CMake SDKs, vcpkg, PyPI, npm, Debian packages
  and a signed HTTPS APT repository.
- Wider Godot, Unity and Unreal support, covering editor workflows and exported
  games across engine versions. See the [engine guides](../integrations/overview.md)
  for the capabilities and prerequisites available today.
- Linux ARM64 support backed by use on physical hardware, and assessment of
  additional platforms.
- Clearer first-use tutorials and diagnostics for missing observations,
  calibration and incompatible package/runtime versions.
- More predictable realtime performance while preserving the documented
  lifecycle and coordinate contracts.
- Better Linux authoring tools alongside the Windows authoring workflow.

Face tracking and additional output/provider integrations need separate designs
and validation before becoming supported products.
