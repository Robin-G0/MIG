"""Build a local, runnable examples archive with sources beside each executable."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "lib"))
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import zipfile

from package_linux import SYSTEM_LIBRARIES, bundle_libraries, copy_tree, create_archive, verify_architecture
from release_metadata import PACKAGE_NAME, PROJECT_NAME, REPOSITORY, release_version, rewrite_package_guides

PROJECT = Path(__file__).resolve().parents[2]


def assemble_sources(root):
    for name in ("common", "python-tkinter", "pygame", "sdl2", "sfml",
                 "unity", "godot", "unreal", "sdk-consumer", "native-consumer"):
        copy_tree(PROJECT / "examples" / name, root / "examples" / name)
    copy_tree(PROJECT / "bindings/python/mig", root / "bindings/python/mig")
    copy_tree(PROJECT / "bindings/dotnet", root / "bindings/dotnet")
    copy_tree(PROJECT / "integrations", root / "integrations")
    copy_tree(PROJECT / "configs", root / "configs")
    copy_tree(PROJECT / "docs", root / "docs")
    shutil.copy2(PROJECT / "examples/README.md", root / "examples/README.md")
    shutil.copy2(PROJECT / "examples/README.fr.md", root / "examples/README.fr.md")
    shutil.copy2(PROJECT / "LICENSE", root / "LICENSE")
    for language in ("", ".fr"):
        guide = (PROJECT / f"examples/README{language}.md").read_text(encoding="utf-8")
        guide = guide.replace("(../", "(")
        for name in ("common", "python-tkinter", "pygame", "sdl2", "sfml",
                     "web", "react", "vue", "next", "godot", "unity", "unreal",
                     "sdk-consumer", "native-consumer"):
            guide = guide.replace(f"({name}/", f"(examples/{name}/")
        (root / f"README{language}.md").write_text(guide, encoding="utf-8")
    (root / "licenses").mkdir()
    shutil.copy2(PROJECT / "examples/common/DejaVuSans-LICENSE", root / "licenses/DejaVuSans-LICENSE")
    copy_tree(PROJECT / "examples/common/licenses", root / "licenses/SDL2_ttf-components")


def assemble_windows(root):
    copy_example_sdk(PROJECT / "build/examples-sdk/windows", root / "sdk")
    runtime = root / "runtime"
    runtime.mkdir()
    source = PROJECT / "build/windows/bin"
    shutil.copy2(source / "libmediapipe.dll", runtime)
    shutil.copy2(PROJECT / "build/windows/src/c-api/Release/mig-c.dll", runtime)
    copy_tree(source / "models", runtime / "models")
    licenses = root / "licenses"
    deps = PROJECT / "build/example-deps"
    for name in ("sdl2", "sfml"):
        directory = root / "examples" / name
        for suffix in ("", "-profile"):
            binary = PROJECT / f"build/examples-compile/windows-{name}/Release/mig-{name}{suffix}.exe"
            shutil.copy2(binary, directory)
    shutil.copy2(deps / "SDL2-2.30.12/lib/x64/SDL2.dll", root / "examples/sdl2")
    shutil.copy2(deps / "SDL2_ttf-2.24.0/lib/x64/SDL2_ttf.dll", root / "examples/sdl2")
    shutil.copy2(deps / "SDL2_ttf-2.24.0/LICENSE.txt", licenses / "SDL2_ttf-LICENSE")
    for name in ("graphics", "window", "system"):
        shutil.copy2(deps / f"SFML-2.6.2/bin/sfml-{name}-2.dll", root / "examples/sfml")
    shutil.copy2(deps / "SDL2-2.30.12/LICENSE.txt", licenses / "SDL2-LICENSE")
    shutil.copy2(deps / "SFML-2.6.2/license.md", licenses / "SFML-LICENSE")
    shutil.copy2(PROJECT / "build/native-deps/MediaPipe-LICENSE", licenses / "MediaPipe-LICENSE")
    shutil.copy2(PROJECT / "build/native-deps/nlohmann-LICENSE", licenses / "nlohmann-LICENSE")
    for binary in root.rglob("*"):
        if binary.suffix not in (".exe", ".dll"):
            continue
        content = binary.read_bytes()
        offset = int.from_bytes(content[60:64], "little")
        if int.from_bytes(content[offset + 4:offset + 6], "little") != 0x8664:
            raise RuntimeError(f"Windows x64 architecture mismatch: {binary}")
    return []


def copy_example_sdk(source, destination):
    """Examples need development libraries, not a second copy of the desktop apps/models."""
    destination.mkdir(parents=True)
    for name in ("include", "lib", "share"):
        if (source / name).is_dir():
            copy_tree(source / name, destination / name)
    if (source / "bin/mig-c.dll").is_file():
        (destination / "bin").mkdir()
        shutil.copy2(source / "bin/mig-c.dll", destination / "bin/mig-c.dll")


def assemble_linux(root):
    copy_example_sdk(PROJECT / "build/release-linux-x64-install", root / "sdk")
    runtime = root / "runtime"
    runtime.mkdir()
    fonts = runtime / "fonts"
    fonts.mkdir()
    shutil.copy2(PROJECT / "examples/common/DejaVuSans.ttf", fonts)
    (runtime / "fonts.conf").write_text(
        '<?xml version="1.0"?>\n<!DOCTYPE fontconfig SYSTEM "fonts.dtd">\n'
        '<fontconfig><dir prefix="relative">fonts</dir>'
        '<cachedir prefix="xdg">fontconfig</cachedir></fontconfig>\n')
    native = PROJECT / "build/native-linux-deps"
    shutil.copy2(native / "libmediapipe.so", runtime)
    shutil.copy2(PROJECT / "build/release-linux-x64/src/c-api" / f"libmig-c.so.{release_version()}",
                 runtime / "libmig-c.so.1")
    copy_tree(native / "models", runtime / "models")
    mediapipe_license = native / "MediaPipe-LICENSE"
    if not mediapipe_license.is_file():
        mediapipe_license = native / "LICENSE"  # older bootstrapped dependency trees
    shutil.copy2(mediapipe_license, root / "licenses/MediaPipe-LICENSE")
    shutil.copy2(native / "nlohmann-LICENSE", root / "licenses/nlohmann-LICENSE")
    seeds = [runtime / "libmediapipe.so", runtime / "libmig-c.so.1"]
    inventory = bundle_libraries(seeds, runtime / "lib", root / "licenses",
                                 SYSTEM_LIBRARIES | {"libstdc++.so.6", "libgcc_s.so.1"})
    plugins = Path("/usr/lib/x86_64-linux-gnu/qt6/plugins")
    plugin_files = []
    for name in ("platforms/libqxcb.so", "platforms/libqoffscreen.so"):
        destination = runtime / "qt/plugins" / name
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(plugins / name, destination)
        plugin_files.append(destination)
    for name in ("sdl2", "sfml"):
        folder = root / "examples" / name
        binaries = []
        for suffix in ("", "-profile"):
            executable = f"mig-{name}{suffix}"
            binary = folder / f"{executable}.bin"
            shutil.copy2(PROJECT / f"build/examples-compile/linux-{name}/{executable}", binary)
            binary.chmod(0o755)
            binaries.append(binary)
            launcher = folder / executable
            launcher.write_text(f'''#!/bin/sh
set -eu
folder=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
export LD_LIBRARY_PATH="$folder/lib:$folder/../../runtime/lib${{LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}}"
export QT_QPA_PLATFORM_PLUGIN_PATH="$folder/../../runtime/qt/plugins"
export FONTCONFIG_FILE="$folder/../../runtime/fonts.conf"
exec "$folder/{executable}.bin" "$@"
''')
            launcher.chmod(0o755)
        inventory.extend(bundle_libraries(binaries + plugin_files, folder / "lib", root / "licenses"))
    verify_architecture(root, "x64")
    return inventory


def assemble_python(root, platform):
    copy_tree(PROJECT / "build/frozen" / platform / "licenses", root / "licenses/python")
    for technology in ("python-tkinter", "pygame"):
        for variant in ("main", "profile"):
            source = PROJECT / "build/frozen" / platform / technology / variant
            if not source.is_dir():
                raise RuntimeError(f"Freeze Python examples before packaging: {source}")
            for item in source.iterdir():
                destination = root / "examples" / technology / item.name
                if item.is_dir():
                    merge_runtime(item, destination)
                else:
                    shutil.copy2(item, destination)


def merge_runtime(source, destination):
    """Share demo/importer dependencies only when same-name bytes are identical."""
    destination.mkdir(parents=True, exist_ok=True)
    for item in source.iterdir():
        target = destination / item.name
        if item.is_symlink():
            if target.is_symlink():
                if os.readlink(target) != os.readlink(item):
                    raise RuntimeError(f"Conflicting frozen symlinks: {target}")
            elif target.exists():
                raise RuntimeError(f"Frozen symlink conflicts with a file: {target}")
            else:
                target.symlink_to(os.readlink(item))
        elif item.is_dir():
            merge_runtime(item, target)
        elif target.exists():
            if target.read_bytes() != item.read_bytes():
                raise RuntimeError(f"Conflicting frozen dependencies: {target}")
        else:
            shutil.copy2(item, target)


def assemble_consumers(root, platform):
    for technology, executable in (("sdk-consumer", "mig-sdk-example"),
                                   ("native-consumer", "mig-native-example")):
        folder = root / "examples" / technology
        argument = "configuration/default.json" if technology == "sdk-consumer" else "../../runtime"
        if platform == "windows-x64":
            source = PROJECT / f"build/examples-compile/windows-{technology}/Release/{executable}.exe"
            shutil.copy2(source, folder)
            (folder / "run.cmd").write_text(
                f'@echo off\ncd /d "%~dp0"\n"%~dp0{executable}.exe" "{argument}"\n', encoding="utf-8")
        else:
            source = PROJECT / f"build/examples-compile/linux-{technology}/{executable}"
            shutil.copy2(source, folder)
            launcher = folder / "run.sh"
            launcher.write_text(f'''#!/bin/sh
set -eu
folder=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
export LD_LIBRARY_PATH="$folder/../../runtime/lib${{LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}}"
export FONTCONFIG_FILE="$folder/../../runtime/fonts.conf"
cd "$folder"
exec "./{executable}" "{argument}"
''')
            launcher.chmod(0o755)


def verify_windows(root):
    for binary in root.rglob("*"):
        if binary.suffix not in (".exe", ".dll", ".pyd"):
            continue
        content = binary.read_bytes()
        offset = int.from_bytes(content[60:64], "little")
        if int.from_bytes(content[offset + 4:offset + 6], "little") != 0x8664:
            raise RuntimeError(f"Windows x64 architecture mismatch: {binary}")


def write_manifest(root, platform, dependencies):
    hashes = {path.relative_to(root).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
              for path in sorted(root.rglob("*")) if path.is_file() and path.name != "manifest.json"}
    (root / "manifest.json").write_text(json.dumps(
        {"project": PROJECT_NAME, "package": PACKAGE_NAME, "repository": REPOSITORY,
         "version": release_version(), "platform": platform, "dependencies": dependencies, "sha256": hashes},
        indent=2) + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--platform", choices=("windows-x64", "linux-x64"), required=True)
    parser.add_argument("--standalone", action="store_true", help="Also package each native example independently")
    options = parser.parse_args()
    root = PROJECT / "build/examples" / options.platform
    if root.exists():
        if root.is_symlink() or root.resolve().parent != (PROJECT / "build/examples").resolve():
            raise RuntimeError("Refusing cleanup outside build/examples")
        shutil.rmtree(root)
    root.mkdir(parents=True)
    assemble_sources(root)
    inventory = assemble_windows(root) if options.platform == "windows-x64" else assemble_linux(root)
    assemble_python(root, options.platform)
    assemble_consumers(root, options.platform)
    if options.platform == "windows-x64":
        verify_windows(root)
    else:
        verify_architecture(root, "x64")
    rewrite_package_guides(root)
    write_manifest(root, options.platform, inventory)
    output = PROJECT / "build/releases"
    output.mkdir(parents=True, exist_ok=True)
    name = f"motion-input-grid-{release_version()}-{options.platform}-examples"
    if options.platform == "windows-x64":
        archive = output / f"{name}.zip"
        with zipfile.ZipFile(archive, "w", compression=zipfile.ZIP_DEFLATED,
                             strict_timestamps=False) as target:
            for file in sorted(root.rglob("*")):
                if file.is_file():
                    target.write(file, f"{name}/{file.relative_to(root).as_posix()}")
        checksum = hashlib.sha256(archive.read_bytes()).hexdigest()
        Path(f"{archive}.sha256").write_text(f"{checksum}  {archive.name}\n")
    else:
        archive = output / f"{name}.tar.gz"
        create_archive(root, archive)
    print(root)
    print(archive)
    if options.standalone:
        for example in ("python-tkinter", "pygame", "sdl2", "sfml", "sdk-consumer", "native-consumer"):
            subprocess.run([sys.executable, str(PROJECT / "tools/packaging/package-single-example.py"),
                            "--platform", options.platform, "--example", example], check=True)


if __name__ == "__main__":
    main()
