"""Package editor integration sources with a matching native SDK."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import tempfile
import zipfile

from package_linux import copy_tree, verify_architecture
from release_metadata import build_directory, release_version

ROOT = Path(__file__).resolve().parents[1]


def assemble(folder, technology, sdk):
    copy_tree(ROOT / "examples" / technology, folder)
    if technology == "godot":
        copy_tree(ROOT / "integrations/godot", folder / "integrations/godot")
    for guide in folder.rglob("README*.md"):
        content = guide.read_text(encoding="utf-8")
        relative_docs = Path(os.path.relpath(folder / "docs", guide.parent)).as_posix()
        for source in ("../../docs/", "../../../docs/"):
            content = content.replace(f"({source}", f"({relative_docs}/")
        guide.write_text(content, encoding="utf-8")
    shutil.copy2(ROOT / "LICENSE", folder / "LICENSE")
    copy_tree(ROOT / "docs", folder / "docs")
    if technology == "unreal":
        copy_tree(sdk, folder / "ThirdParty")
    else:
        bridge_folder = folder / "csharp" if technology == "godot" else folder
        for name in ("MigTracker.cs", "SyntheticFrames.cs"):
            shutil.copy2(ROOT / "bindings/dotnet" / name, bridge_folder / name)
        copy_tree(sdk, folder / "sdk")
    hashes = {file.relative_to(folder).as_posix(): hashlib.sha256(file.read_bytes()).hexdigest()
              for file in sorted(folder.rglob("*")) if file.is_file()}
    (folder / "manifest.json").write_text(json.dumps({"sha256": hashes}, indent=2) + "\n")


def package(technology, platform, sdk):
    releases = ROOT / "build/releases"
    name = f"mig-{release_version()}-{platform}-{technology}-editor-sources"
    releases.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="mig-editor-", dir=build_directory()) as temporary:
        folder = Path(temporary) / name
        assemble(folder, technology, sdk)
        archive = releases / f"{name}.zip"
        with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED, strict_timestamps=False) as target:
            for file in sorted(folder.rglob("*")):
                if file.is_file():
                    target.write(file, f"{name}/{file.relative_to(folder).as_posix()}")
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    Path(f"{archive}.sha256").write_text(f"{digest}  {archive.name}\n")
    print(archive)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk", type=Path, required=True)
    parser.add_argument("--platform", choices=("windows-x64", "linux-x64", "linux-arm64"), required=True)
    options = parser.parse_args()
    sdk = options.sdk.resolve()
    if not (sdk / "include/mig/c/api.h").is_file():
        parser.error("The SDK must include the shared C API")
    if options.platform.startswith("linux-"):
        verify_architecture(sdk, options.platform.removeprefix("linux-"))
    else:
        library = sdk / "bin/mig-c.dll"
        content = library.read_bytes()
        offset = int.from_bytes(content[60:64], "little")
        if int.from_bytes(content[offset + 4:offset + 6], "little") != 0x8664:
            parser.error("Windows x64 SDK architecture mismatch")
    for technology in ("unity", "godot", "unreal"):
        package(technology, options.platform, sdk)


if __name__ == "__main__":
    main()
