# C++ native inference consumer

[English](README.md) | [Français](README.fr.md)

Requires the installed native SDK and bootstrapped MediaPipe runtime/models.
`demonstrate_inference(runtime, hands)` owns one estimator and performs inference
on blank RGB, demonstrating explicit hand capability and RAII cleanup.

```sh
cmake -S examples/native-consumer -B build/native-example -DCMAKE_PREFIX_PATH=/absolute/path/to/sdk
cmake --build build/native-example --config Release
build/native-example/mig-native-example /absolute/path/to/runtime --hands
```

Windows uses `Release/mig-native-example.exe`. Omit `--hands` for a body-only build.
Replace blank RGB with your capture frame and preserve width, height, timestamp and
sequence. A `Pose` and its borrowed results belong to one thread. Copy hand results
before the next inference call. For a complete capture loop, see `sdl2`/`sfml`.

[Complete source walkthrough](../../docs/getting-started/examples.md) · [Bootstrap](../../docs/getting-started/bootstrap.md).
