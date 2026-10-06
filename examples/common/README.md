# Shared example helpers

[English](README.md) | [Français](README.fr.md)

These helpers support the examples; they are not recognition-library internals.
Each has one responsibility and uses the installed public SDK.

| File | Reusable entry point |
| --- | --- |
| options.hpp | `Options(argc, argv)`: explicit synthetic/runtime selection |
| runtime.hpp | `find_runtime()`: bundled or checkout runtime with platform library and models |
| source.hpp | `Source::sample()`: camera/model ownership and copied observations |
| synthetic.hpp | `synthetic_frame()`: deterministic default-profile body/hand fixture |
| recognition.hpp | `consume()`: logical action callback |
| drawing.hpp | `draw(source, line, dot)`: renderer-independent overlay traversal |
| profile.hpp | Atomic profile replacement, file picker and portable font assets |
| python_source.py | `InputSource.take()/close()`: bounded worker mailbox for GUI hosts |

Python profile imports are queued to the camera owner thread. Failed validation
preserves the running profile; successful import restarts capture/calibration.
Demo and profile variants share recognition and rendering instead of duplicating
camera loops. SDL2/SFML HUDs cache their text until the accepted actions change.

`licenses/` contains the FreeType, HarfBuzz and zlib notices from the official
SDL2_ttf 2.24.0 source archive's VisualC/external/lib/x64 folder (SHA256
`0b2bf1e7b6568adbdbc9bb924643f79d9dedafe061fa1ed687d1d9ac4e453bfd`).
DejaVuSans.ttf is redistributable under the adjacent DejaVuSans-LICENSE.

Copy or adapt only the helpers you need. Real-time games may replace synchronous
C++ capture with a worker, as the Python GUIs demonstrate. Keep engine recognition
in MIG, display mirroring in rendering, and camera ownership in one thread.

[Complete source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).
