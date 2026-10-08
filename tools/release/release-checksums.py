#!/usr/bin/env python3
"""Write or verify checksums for local release artifacts; never publish them."""
import argparse
import hashlib
from pathlib import Path


def digest(path):
    checksum = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            checksum.update(block)
    return checksum.hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path, nargs="?", default=Path("build/releases"))
    parser.add_argument("--check", action="store_true")
    options = parser.parse_args()
    manifest = options.directory / "SHA256SUMS"
    if options.check:
        for line in manifest.read_text().splitlines():
            expected, name = line.split("  ", 1)
            if Path(name).name != name or digest(options.directory / name) != expected:
                raise RuntimeError(f"Checksum mismatch: {name}")
        print("Release checksums verified")
        return
    artifacts = [path for path in sorted(options.directory.iterdir())
                 if path.is_file() and path.name.endswith((".zip", ".tar.gz", ".tgz", ".whl", ".deb"))]
    manifest.write_text("".join(f"{digest(path)}  {path.name}\n" for path in artifacts))
    print(manifest)


if __name__ == "__main__":
    main()
