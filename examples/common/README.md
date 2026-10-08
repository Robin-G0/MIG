# Canonical example assets and repository fixtures

[English](README.md) | [Français](README.fr.md)

Runnable tutorials own their integration in `example_usage` and their local
`support/` helpers. Copy a complete tutorial folder; it does not need this directory.

| File/directory | Purpose |
| --- | --- |
| `raised-hands.json` | Canonical two-wrist profile used to prepare camera/browser assets. |
| `synthetic.hpp` | Deterministic body/hand fixture for repository tests. |
| `sdk.cmake` | Full-repository examples build: installed SDK or checkout fallback. |
| `DejaVuSans.ttf`, `DejaVuSans-LICENSE` | Redistributable font and its license. |
| `licenses/` | FreeType, HarfBuzz and zlib notices for SDL2_ttf components. |

The font/notices come from the official SDL2_ttf 2.24.0 dependencies. The source
archive SHA256 is `0b2bf1e7b6568adbdbc9bb924643f79d9dedafe061fa1ed687d1d9ac4e453bfd`.
Local tutorial profiles/support assets are included explicitly so they survive
copying outside this repository. Update the matching tutorial files when changing
canonical fixtures/assets; the example and extracted-package tests check behavior.

[Tutorial index](../README.md) · [Source walkthrough](../../docs/getting-started/examples.md).
