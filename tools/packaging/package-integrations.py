"""Build runtime-only Godot, Unity and Unreal packages from an installed SDK."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "lib"))
import argparse
import hashlib
import shutil
import subprocess
import tempfile
import zipfile

from package_linux import copy_tree, create_archive, verify_architecture
from release_metadata import build_directory, ROOT, release_version, write_package_manifest, rewrite_package_guides


def copy_runtime(sdk, destination, platform):
    destination.mkdir(parents=True, exist_ok=True)
    name = "mig-c.dll" if platform.startswith("windows") else "libmig-c.so.1"
    matches = list(sdk.rglob(name))
    if len(matches) != 1:
        raise ValueError(f"Expected one {name} in {sdk}, found {len(matches)}")
    shutil.copy2(matches[0].resolve(), destination / name)
    if platform.startswith("linux"):
        verify_architecture(destination, platform.split("-")[1])


def copy_licenses(destination, dependencies):
    destination.mkdir(parents=True, exist_ok=True)
    shutil.copy2(ROOT / "LICENSE", destination / "LICENSE")
    shutil.copy2(dependencies / "nlohmann-LICENSE", destination / "nlohmann-LICENSE")


def unity_plugin_metadata(path, platform):
    guid = hashlib.md5(f"MIG/{platform}".encode(), usedforsecurity=False).hexdigest()
    os_name = "Windows" if platform.startswith("windows") else "Linux"
    player = "Win64" if platform.startswith("windows") else "Linux64"
    cpu = "x86_64" if platform.endswith("x64") else "ARM64"
    path.with_name(path.name + ".meta").write_text(
        f"fileFormatVersion: 2\nguid: {guid}\nPluginImporter:\n"
        "  serializedVersion: 2\n  isPreloaded: 0\n  isOverridable: 0\n"
        "  platformData:\n  - first:\n      Any: \n    second:\n      enabled: 0\n"
        f"  - first:\n      Editor: Editor\n    second:\n      enabled: 1\n"
        f"      settings:\n        CPU: {cpu}\n        OS: {os_name}\n        DefaultValueInitialized: true\n"
        f"  - first:\n      Standalone: {player}\n    second:\n      enabled: 1\n"
        f"      settings:\n        CPU: {cpu}\n", encoding="utf-8")


def stage_unity(folder, sdk, platform):
    copy_tree(ROOT / "integrations/unity", folder)
    bridge = folder / "Runtime/Bridge"
    bridge.mkdir()
    shutil.copy2(ROOT / "bindings/dotnet/MigTracker.cs", bridge / "MigTracker.cs")
    plugins = folder / "Runtime/Plugins" / platform
    copy_runtime(sdk, plugins, platform)
    if platform.startswith("linux"):
        (plugins / "libmig-c.so.1").rename(plugins / "libmig-c.so")
    for library in plugins.iterdir():
        unity_plugin_metadata(library, platform)


def unity_asset_metadata(folder):
    for asset in sorted(folder.rglob("*")):
        if asset.suffix == ".meta":
            continue
        metadata = asset.with_name(asset.name + ".meta")
        if metadata.exists():
            continue
        relative = asset.relative_to(folder).as_posix()
        guid = hashlib.md5(f"MIG/Unity/{relative}".encode(), usedforsecurity=False).hexdigest()
        header = f"fileFormatVersion: 2\nguid: {guid}\n"
        if asset.is_dir():
            header += "folderAsset: yes\n"
        importers = {".cs": "MonoImporter", ".asmdef": "AssemblyDefinitionImporter",
                     ".json": "TextScriptImporter"}
        importer = importers.get(asset.suffix, "DefaultImporter")
        settings = ""
        if asset.suffix == ".cs":
            settings = "  serializedVersion: 2\n  defaultReferences: []\n  executionOrder: 0\n"
        metadata.write_text(header + f"{importer}:\n  externalObjects: {{}}\n" + settings
                            + "  userData: \n  assetBundleName: \n  assetBundleVariant: \n", encoding="utf-8")


def stage_unreal(folder, sdk, platform):
    copy_tree(ROOT / "integrations/unreal", folder)
    copy_tree(sdk / "include/mig/c", folder / "ThirdParty/include/mig/c")
    if platform.startswith("windows"):
        copy_runtime(sdk, folder / "ThirdParty/bin", platform)
        (folder / "ThirdParty/lib").mkdir()
        shutil.copy2(sdk / "lib/mig-c.lib", folder / "ThirdParty/lib/mig-c.lib")
    else:
        libraries = folder / "ThirdParty/lib"
        copy_runtime(sdk, libraries, platform)


def stage_godot(folder, sdk, platform, bridge, godot_cpp):
    copy_tree(ROOT / "integrations/godot/addons", folder / "addons")
    addon = folder / "addons/mig"
    for guide in (ROOT / "integrations/godot").glob("README*.md"):
        shutil.copy2(guide, addon / guide.name)
    binary = addon / "bin"
    copy_runtime(sdk, binary, platform)
    suffix = "dll" if platform.startswith("windows") else "so"
    shutil.copy2(bridge, binary / f"mig-godot.{suffix}")
    if platform.startswith("linux"):
        verify_architecture(binary, platform.split("-")[1])
    shutil.copy2(godot_cpp / "LICENSE.md", addon / "godot-cpp-LICENSE.md")


def package(args):
    args.destination.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="integration-", dir=build_directory()) as temporary:
        name = f"motion-input-grid-{release_version()}-{args.platform}-{args.ecosystem}"
        folder = Path(temporary) / ("package" if args.ecosystem == "unity" else name)
        if args.ecosystem == "unity":
            stage_unity(folder, args.sdk, args.platform)
        elif args.ecosystem == "unreal":
            stage_unreal(folder, args.sdk, args.platform)
        else:
            stage_godot(folder, args.sdk, args.platform, args.godot_bridge, args.godot_cpp)
        license_folder = folder / "addons/mig" if args.ecosystem == "godot" else folder
        copy_licenses(license_folder / "licenses", args.dependencies)
        shutil.copy2(ROOT / "LICENSE", license_folder / "LICENSE")
        rewrite_package_guides(folder)
        if args.ecosystem == "unity":
            unity_asset_metadata(folder)
        write_package_manifest(folder, args.ecosystem, args.platform)
        if args.ecosystem == "unity":
            archive = args.destination / f"{name}.tgz"
            create_archive(folder, archive)
        else:
            archive = args.destination / f"{name}.zip"
            with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as output:
                for file in sorted(folder.rglob("*")):
                    if file.is_file():
                        relative = file.relative_to(folder).as_posix()
                        output.write(file, relative if args.ecosystem == "godot" else f"MIG/{relative}")
        print(archive)
        examples = (('godot-gdscript', 'godot-csharp') if args.platform.endswith('x64')
                    else ('godot-gdscript',)) if args.ecosystem == 'godot' else (args.ecosystem,)
        for ecosystem in examples:
            subprocess.run([sys.executable, str(ROOT / 'tools/packaging/package-editor-examples.py'),
                            '--integration', str(folder), '--ecosystem', ecosystem,
                            '--platform', args.platform, '--destination', str(args.destination)], check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ecosystem", choices=("godot", "unity", "unreal"), required=True)
    parser.add_argument("--sdk", type=Path, required=True)
    parser.add_argument("--platform", choices=("windows-x64", "linux-x64", "linux-arm64"), required=True)
    parser.add_argument("--dependencies", type=Path, required=True)
    parser.add_argument("--destination", type=Path, default=ROOT / "build/releases")
    parser.add_argument("--godot-bridge", type=Path)
    parser.add_argument("--godot-cpp", type=Path)
    args = parser.parse_args()
    if args.ecosystem == "godot" and (not args.godot_bridge or not args.godot_cpp):
        parser.error("Godot requires --godot-bridge and --godot-cpp")
    if args.ecosystem == "unity" and args.platform.endswith("arm64"):
        parser.error("The current Unity desktop native plugin targets x64 only")
    package(args)


if __name__ == "__main__":
    main()
