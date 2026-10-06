"""Generate the engine source release and a vcpkg overlay pinned to its SHA512."""
import argparse
from pathlib import Path
import shutil
import tempfile

from package_linux import copy_tree, create_archive
from release_metadata import build_directory, ROOT, release_version, file_hash, rewrite_package_guides


def package(destination, source_url=None):
    version = release_version()
    destination.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="source-", dir=build_directory()) as temporary:
        folder = Path(temporary) / f"mig-{version}-source"
        folder.mkdir()
        for name in ("CMakeLists.txt", "VERSION", "LICENSE"):
            shutil.copy2(ROOT / name, folder / name)
        for name in ("src", "cmake"):
            copy_tree(ROOT / name, folder / name)
        archive = destination / f"{folder.name}.tar.gz"
        create_archive(folder, archive)
        overlay = Path(temporary) / "mig-vcpkg-overlay"
        copy_tree(ROOT / "ports/mig", overlay / "mig")
        shutil.copy2(ROOT / "LICENSE", overlay / "LICENSE")
        rewrite_package_guides(overlay)
        url = source_url or f"https://github.com/Robin-G0/MIG/releases/download/v{version}/{archive.name}"
        (overlay / "mig/source.cmake").write_text(
            f'set(MIG_SOURCE_URL "{url}")\n'
            f'set(MIG_SOURCE_SHA512 "{file_hash(archive, "sha512")}")\n', encoding="utf-8")
        create_archive(overlay, destination / f"mig-{version}-vcpkg-overlay.tar.gz")
    print(archive)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--destination", type=Path, default=ROOT / "build/releases")
    parser.add_argument("--source-url")
    args = parser.parse_args()
    package(args.destination, args.source_url)
