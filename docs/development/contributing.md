# Contributing and readability

[English](contributing.md) | [Français](contributing.fr.md)

Code and documentation must be easy for a human to read, modify and review.
C++ uses four spaces, never tabs; `.clang-format` is the formatting authority.
Use descriptive names, explicit ownership, braces and one responsibility per
function/module. Comments explain intent, contracts and tradeoffs, not the syntax.
Avoid compressed statements, speculative abstraction and optimisations that hide
semantics. Keep the portable engine independent of Windows, cameras and ML SDKs.

Format first-party C++ (never downloaded dependencies) with clang-format 16+.
Run `tools/format-code.ps1` to format or `tools/format-code.ps1 -Check` to verify.
Build with `tools/build-windows.ps1 -SkipBootstrap` and run CTest. Changes involving
device ownership need opt-in repeated camera start/stop checks too. Test both
`MIG_BUILD_HANDS=ON` and `OFF` when touching optional inference. Do not equate
successful smoke tests with proof of absence of leaks or human gesture accuracy.

Commit subjects begin with `[ADD]`, `[FIX]` or `[DEL]`, according to the purpose.
Separate formatting, behaviour, tests and documentation where practical. Update
the implementation contracts with meaningful changes.

Use [the current pipeline](../architecture/lifecycle.md) to find each module's responsibility.
Keep documentation readable and distinguish implemented behaviour from plans.
Profile changes invalidate in-flight inference using the revision barrier; do
not bypass it or reintroduce polling, unbounded queues or unsynchronised buffers.

## Continuous integration

[GitHub Actions](../../.github/workflows/ci.yml) runs on every push and pull request,
and can also be started manually from the Actions tab:

- C++ formatting: clang-format 16 checks first-party `.cpp`/`.hpp`/`.h` in `src`,
  `tests` and `examples`, using `.clang-format` (four spaces, no tabs, braces, etc.).
  It reports discrepancies without rewriting or committing files. Other file
  formats are not checked by clang-format. `tools/check-docs.py` also checks
  translation pairs, language switches and local guide links. The release metadata
  check verifies matching versions across native, Python, npm and editor packages.
- Windows Release: bootstrap pinned native dependencies, compile both programs
  and libraries, then run CTest with hands ON and OFF in separate clean jobs.
  Install the SDK and compile both independent consumers to verify exports.
- Linux Release: compile/test/install the portable SDK without Windows/MediaPipe,
  then build and run the independent positions consumer.
- Linux native: Qt6 configurator/controller offscreen checks, native inference,
  shared C ABI and Python/Pygame/Tkinter smoke tests, installed SDL2/SFML consumers.
- Web: Emscripten recognition/JSON/coordinate tests and packaged browser integration.
- JavaScript: React/Vue lifecycle and public types, all framework production builds,
  six Chromium pages using real WASM with synthetic camera observations, npm and
  compiled-example archives. Depends on the Web job's checked assets.

CI does not access a webcam, send keyboard inputs, publish releases or use secrets.
Native tests infer on blank frames; upstream MediaPipe can still attempt metrics
egress. CI does not establish leak freedom or human gesture accuracy.
Checkout is pinned to a commit and workflow permissions are read-only.
To block merges on failures, mark these checks as required in the repository's
branch protection/ruleset settings; merely adding this workflow does not do so.

## Windows dependency downloads

The example and native bootstrap scripts share `tools/download.ps1`. Network
failures, HTTP 408/429 and server errors have at most four attempts, with 90-second
request timeouts and delays of 2, 4 and 8 seconds. Other HTTP client errors fail
immediately. Downloads use a temporary file and enter the cache only after the
SHA256 check, or a nonempty check for unpinned headers/licenses. Invalid cached
artifacts are downloaded again. Checksum failures still stop the build.

`powershell -NoProfile -ExecutionPolicy Bypass -File tests/download_tests.ps1`
checks recovery and integrity failures without using the network. Both Windows
CI and release jobs run it before bootstrap. Only downloads are retried; failed
builds and tests are never turned into successful runs by retries.
