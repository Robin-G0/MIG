"""Archive an installed SDK without applications, estimators or demo runtimes."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "lib"))
import argparse
import shutil
import tempfile
import zipfile

from package_linux import copy_tree, create_archive, verify_architecture
from release_metadata import build_directory, ROOT, release_version, write_package_manifest


def package(args):
    args.destination.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="sdk-", dir=build_directory()) as temporary:
        name = f"motion-input-grid-{release_version()}-{args.platform}-sdk"
        folder = Path(temporary) / name
        sdk = folder / "sdk"
        for directory in ("include", "lib", "share"):
            if (args.sdk / directory).is_dir():
                copy_tree(args.sdk / directory, sdk / directory)
        if args.platform.startswith("windows"):
            (sdk / "bin").mkdir()
            shutil.copy2(args.sdk / "bin/mig-c.dll", sdk / "bin/mig-c.dll")
        if (sdk / "include/mig/native").exists() or list(sdk.rglob("*mig-native*")):
            raise ValueError("Build a positions-only SDK before packaging it")
        licenses = sdk / "share/licenses/mig"
        licenses.mkdir(parents=True, exist_ok=True)
        shutil.copy2(ROOT / "LICENSE", licenses / "LICENSE")
        shutil.copy2(args.dependencies / "nlohmann-LICENSE", licenses / "nlohmann-LICENSE")
        if args.platform.startswith("linux"):
            verify_architecture(folder, args.platform.split("-")[1])
        write_package_manifest(folder, "cmake", args.platform)
        if args.platform.startswith("windows"):
            archive = args.destination / f"{name}.zip"
            with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as target:
                for file in sorted(folder.rglob("*")):
                    if file.is_file():
                        target.write(file, f"{name}/{file.relative_to(folder).as_posix()}")
        else:
            create_archive(folder, args.destination / f"{name}.tar.gz")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sdk", type=Path, required=True)
    parser.add_argument("--dependencies", type=Path, required=True)
    parser.add_argument("--platform", choices=("windows-x64", "linux-x64", "linux-arm64"), required=True)
    parser.add_argument("--destination", type=Path, default=ROOT / "build/releases")
    package(parser.parse_args())
