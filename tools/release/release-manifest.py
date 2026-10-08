"""Inventory a fresh candidate directory without publishing its artifacts."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "lib"))
import argparse
import json
import re

from release_metadata import release_version, file_hash, PROJECT_NAME, PACKAGE_NAME, REPOSITORY


def generate(directory, tag=None):
    version = release_version(tag)
    entries = []
    for file in sorted(directory.iterdir()):
        ignored = file.name in ("SHA256SUMS", "release-manifest.json") or file.suffix == ".sha256"
        if not file.is_file() or ignored:
            continue
        if not file.name.startswith((PACKAGE_NAME + "-", PACKAGE_NAME + "_", "motion_input_grid-")):
            raise ValueError(f"Unexpected package identifier: {file.name}")
        if not re.search(rf"(?<![0-9]){re.escape(version)}(?![0-9]|\.[0-9])", file.name):
            raise ValueError(f"Unversioned or stale artifact in candidate directory: {file.name}")
        entries.append({"name": file.name, "bytes": file.stat().st_size, "sha256": file_hash(file)})
    if not entries:
        raise ValueError("No release artifacts found")
    manifest = {
        "project": PROJECT_NAME, "package": PACKAGE_NAME, "repository": REPOSITORY,
        "version": version, "tag": tag or f"v{version}", "artifacts": entries,
        "publication": "manual",
        "support_matrix": f"{REPOSITORY}/blob/v{version}/docs/reference/support.md",
    }
    (directory / "release-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    checksums = [(entry["name"], entry["sha256"]) for entry in entries]
    checksums.append(("release-manifest.json", file_hash(directory / "release-manifest.json")))
    (directory / "SHA256SUMS").write_text("".join(f"{digest}  {name}\n" for name, digest in checksums))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("--tag")
    args = parser.parse_args()
    generate(args.directory, args.tag)
