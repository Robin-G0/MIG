# C++ positions-only consumer

[English](README.md) | [Français](README.fr.md)

To install just the library and link it to your project, follow the
[C++ SDK / CMake guide](../../docs/getting-started/cpp.md). It covers archives, source builds,
`cmake --install` and generated Makefiles on Linux.

Requires CMake 3.25, C++20 and an installed MIG SDK with `MIG::core`/`MIG::format`.
No camera, models, Python or window system is needed.

```sh
cmake -S examples/sdk-consumer -B build/sdk-example -DCMAKE_PREFIX_PATH=/absolute/path/to/sdk
cmake --build build/sdk-example --config Release
build/sdk-example/mig-sdk-example configs/default.json
```

Visual Studio places the executable under `Release/`. The supplied profile emits
`left_raise`. `calibrate()` supplies stable synthetic shoulders; `demonstrate_path()`
walks one simple authored path; `dispatch_events()` shows the application callback.
Replace the synthetic frame construction with your estimator and retain
`engine.update(frame, frame.timestamp_ms)`. The synthetic walker is intentionally
not a simulator for arbitrary simultaneous/finger/Interaction profiles.

[Complete source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).
