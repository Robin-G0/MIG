# Roadmap

[English](ROADMAP.md) | [Français](ROADMAP.fr.md)

Priorities follow validation and user feedback; no delivery dates are promised.
Godot, Unity and Unreal stay **Preview** until editor and exported-game testing
covers enough supported versions/platforms. Current evidence is in the [support matrix](../reference/support.md).

- [ ] Test integrated and USB webcams on multiple machines, CPU/GPU configurations,
  resolutions and frame rates, including tracking loss and held-key release.
- [ ] Verify clean installs from published CMake SDK, vcpkg download, PyPI, npm,
  Debian packages and signed HTTPS APT repository after their first upload.
- [ ] Validate Godot imports, editor UI, exports and multiple Godot versions.
- [ ] Validate Unity local/Git/tarball UPM installs, editor/play mode and player builds.
- [ ] Compile Unreal modules, test Blueprint events, packaged games and engine versions.
- [ ] Run Linux ARM64 packages on physical hardware; assess other platforms separately.
- [ ] Gather beginner/game-jam and production integration feedback; simplify first use.
- [ ] Improve diagnostics for missing observations, calibration and package/runtime mismatch.
- [ ] Add regressions for real reported failures; preserve lifecycle and coordinate contracts.
- [ ] Profile realtime behavior on representative workloads before further optimizations.
- [ ] Improve developer experience and Linux authoring parity without duplicating the engine.

Face tracking and additional output/provider integrations need separate designs
and validation before becoming supported products.
