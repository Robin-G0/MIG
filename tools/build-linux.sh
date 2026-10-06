#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
build_directory="${1:-$project_root/build/linux-apps}"
hands="${2:-ON}"
if [[ "$hands" != ON && "$hands" != OFF ]]; then
    echo 'Hands must be ON or OFF.' >&2
    exit 1
fi

python3 "$project_root/tools/bootstrap-native-linux.py"
cmake -S "$project_root" -B "$build_directory" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release -DMIG_BUILD_HANDS="$hands" \
    -DMIG_BUILD_CONFIGURATOR=ON -DMIG_BUILD_CONTROLLER=ON \
    -DMIG_NATIVE_DEPS="$project_root/build/native-linux-deps"
cmake --build "$build_directory" --parallel "${MIG_BUILD_JOBS:-3}"
ctest --test-dir "$build_directory" --output-on-failure
