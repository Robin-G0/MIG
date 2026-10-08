#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$project_root"
sdk="${MIG_EXAMPLE_SDK:-$project_root/build/release-linux-x64-install}"
for example in sdl2 sfml; do
    build="build/examples-compile/linux-$example"
    cmake -S "examples/$example" -B "$build" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$sdk"
    cmake --build "$build" --parallel "${MIG_BUILD_JOBS:-3}"
    SDL_VIDEODRIVER=dummy "$build/mig-$example" --smoke
    SDL_VIDEODRIVER=dummy "$build/mig-$example-profile" --smoke
done
for example in sdk-consumer native-consumer; do
    build="build/examples-compile/linux-$example"
    cmake -S "examples/$example" -B "$build" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$sdk"
    cmake --build "$build" --parallel "${MIG_BUILD_JOBS:-3}"
done
python3 tools/freeze-python-examples.py
python3 tools/package-examples.py --platform linux-x64 --standalone
xvfb-run -a python3 tests/packaging/standalone_example_tests.py --source-python "$(command -v python3)" 'build/releases/*-linux-x64-*-standalone.tar.gz'
