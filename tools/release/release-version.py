"""Reject release tags and package metadata that disagree with VERSION."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "lib"))
import argparse
import runpy

from release_metadata import ROOT, release_version, synchronize_versions


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tag", help="Validate the resolved version against this tag; does not set it")
    parser.add_argument("--sync", action="store_true", help="Propagate the resolved tag/fallback version")
    args = parser.parse_args()
    release_version(args.tag)
    if args.sync:
        synchronize_versions()
    runpy.run_path(str(ROOT / "tests/tooling/release_version_tests.py"), run_name="__main__")
