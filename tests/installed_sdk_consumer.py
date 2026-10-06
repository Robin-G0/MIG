"""Shared strict CMake consumer for extracted SDK and vcpkg artifacts."""
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def verify_consumer(sdk, folder, arm64=False):
    source = folder / "consumer"
    source.mkdir()
    version = (ROOT / "VERSION").read_text().strip()
    code = (ROOT / "examples/sdk-consumer/main.cpp").read_text()
    code = '#include <mig/core/library.hpp>\n#include <mig/c/api.h>\n' + code
    entry = "int main(int argc, char** argv) {"
    assert entry in code
    code = code.replace(entry, entry + f'\n    if (mig::core::library_version() != "{version}" || '
                        'mig_abi_version() != 1 || mig_packet_size() != sizeof(mig_packet)) {\n'
                        '        return 1;\n    }\n')
    (source / "main.cpp").write_text(code)
    (source / "CMakeLists.txt").write_text(
        'cmake_minimum_required(VERSION 3.25)\nproject(InstalledConsumer LANGUAGES CXX)\n'
        f'find_package(MIG {version} EXACT CONFIG REQUIRED)\nadd_executable(consumer main.cpp)\n'
        'target_link_libraries(consumer PRIVATE MIG::core MIG::format MIG::hands MIG::c)\n')
    build = folder / "consumer-build"
    configure = ["cmake", "-S", str(source), "-B", str(build),
                 f"-DCMAKE_PREFIX_PATH={sdk}"]
    if os.name != "nt":
        configure.append("-DCMAKE_BUILD_TYPE=Release")
    if arm64:
        configure.append(f"-DCMAKE_TOOLCHAIN_FILE={ROOT / 'cmake/linux-arm64.cmake'}")
    subprocess.run(configure, check=True)
    subprocess.run(["cmake", "--build", str(build), "--config", "Release", "--parallel", "3"], check=True)
    binary = build / ("Release/consumer.exe" if os.name == "nt" else "consumer")
    command = [str(binary), str(ROOT / "configs/default.json")]
    environment = os.environ.copy()
    if os.name == "nt":
        environment["PATH"] = str(sdk / "bin") + os.pathsep + environment["PATH"]
    if arm64:
        command = ["qemu-aarch64", "-L", "/usr/aarch64-linux-gnu", *command]
    result = subprocess.check_output(command, env=environment, text=True)
    assert "Game event: left_raise" in result, result
    print(f"External CMake consumer: C++/C ABI, engine {version} and recognition passed")
