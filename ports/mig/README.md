# MIG vcpkg overlay

[English](README.md) | [Français](README.fr.md)

This directory is the port for MIG itself. The root `vcpkg.json` is the separate
dependency manifest used to build MIG.

Run `python tools/package-source.py` to create the engine source archive and the
overlay archive in `build/releases`. Extract the overlay and pass its root to
`vcpkg install mig --overlay-ports=/path/to/mig-vcpkg-overlay`. The generated
`mig/source.cmake` pins the release URL and the source archive's actual SHA512.
The tracked template intentionally fails until that pin has been generated.

Default features provide JSON profiles and hands. `mig[c-api]` adds the shared
C ABI and requires a dynamic triplet. Applications and MediaPipe are excluded.
Consumers use `find_package(MIG CONFIG REQUIRED)` and `MIG::core`, `MIG::format`,
`MIG::hands` or `MIG::c`. Use the same compiler/toolset for the SDK and consumer.

Before uploading the source archive, the artifact test seeds vcpkg's download
cache with the generated file and still validates the pinned hash. After manual
upload, test the public release URL without that cache. Publishing a registry port
or submitting to upstream vcpkg remains manual.

[Distribution guide](../../docs/development/distribution.md).
