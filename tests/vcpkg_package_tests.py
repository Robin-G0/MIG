"""Build a generated vcpkg overlay and consume its installed CMake and C ABI exports."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile
import tempfile
from installed_sdk_consumer import verify_consumer

ROOT = Path(__file__).resolve().parents[1]


def verify(overlay, source, executable):
    with tempfile.TemporaryDirectory(prefix="vcpkg-artifact-") as temporary:
        folder = Path(temporary)
        with tarfile.open(overlay) as archive:
            archive.extractall(folder, filter="data")
        (folder / "vcpkg.json").write_text('{"dependencies":[{"name":"mig","features":["c-api"]}]}\n')
        downloads = folder / "downloads"
        downloads.mkdir()
        shutil.copy2(source, downloads / source.name)
        triplet = "x64-windows" if os.name == "nt" else "x64-linux-dynamic"
        command = [str(executable.resolve()), "install", f"--x-manifest-root={folder}",
                   f"--triplet={triplet}", f"--overlay-ports={folder}/mig-vcpkg-overlay",
                   f"--x-install-root={folder}/installed", f"--x-buildtrees-root={folder}/buildtrees",
                   f"--x-packages-root={folder}/packages", f"--downloads-root={downloads}", "--binarysource=clear"]
        subprocess.run(command, cwd=folder, check=True)
        sdk = folder / "installed" / triplet
        verify_consumer(sdk, folder)
        library = sdk / ("bin/mig-c.dll" if os.name == "nt" else "lib/libmig-c.so.1")
        subprocess.run([sys.executable, str(ROOT / "tests/python_tests.py"), str(library)], check=True)
        print("Generated vcpkg overlay: install, external CMake consumer and C ABI recognition passed")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("overlay", type=Path)
    parser.add_argument("source", type=Path)
    parser.add_argument("--vcpkg", type=Path, required=True)
    args = parser.parse_args()
    verify(args.overlay, args.source, args.vcpkg)
