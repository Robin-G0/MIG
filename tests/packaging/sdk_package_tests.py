"""Extract a generated SDK and build a strict external find_package consumer."""
import argparse
from pathlib import Path
import tarfile
import tempfile
import zipfile
from installed_sdk_consumer import verify_consumer

ROOT = Path(__file__).resolve().parents[2]


def verify(archive, emulator=None):
    with tempfile.TemporaryDirectory(prefix="sdk-test-") as temporary:
        folder = Path(temporary)
        if zipfile.is_zipfile(archive):
            with zipfile.ZipFile(archive) as package:
                package.extractall(folder)
        else:
            with tarfile.open(archive) as package:
                package.extractall(folder, filter="data")
        sdk = next(folder.glob("*/sdk"))
        verify_consumer(sdk, folder, emulator)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    parser.add_argument("--arm64", action="store_true")
    args = parser.parse_args()
    verify(args.archive, args.arm64)
