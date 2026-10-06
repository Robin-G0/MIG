"""Audit source files for private preparation content, generated payloads and credentials."""
import argparse
import os
from pathlib import Path
import re
import sys

sys.dont_write_bytecode = True

from distribution_policy import GENERATED_DIRECTORIES, GENERATED_PATHS, private_file

SECRET_PATTERNS = (
    r"-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----",
    r"\b(?:ghp|gho|ghs|github_pat)_[A-Za-z0-9_]{30,}\b",
    r"\bAKIA[A-Z0-9]{16}\b",
)
LOCAL_PATH = re.compile(r"[A-Za-z]:[\\/]Users[\\/]|/home/(?!runner(?:/|$))[A-Za-z0-9_.-]+/")
BINARY_SUFFIXES = {".exe", ".dll", ".so", ".a", ".lib", ".pdb", ".zip", ".tgz", ".whl", ".deb"}


def audit(root, public=False):
    failures = []
    count = 0
    for directory, folders, names in os.walk(root, followlinks=False):
        for name in list(folders):
            path = Path(directory) / name
            if name in GENERATED_DIRECTORIES or path.relative_to(root).as_posix() in GENERATED_PATHS or path.is_symlink():
                if public:
                    failures.append(f"Excluded directory: {path.relative_to(root)}")
                folders.remove(name)
        for name in names:
            path = Path(directory) / name
            relative = path.relative_to(root)
            if private_file(name) or relative.as_posix() in GENERATED_PATHS:
                if public:
                    failures.append(f"Private file: {relative}")
                continue
            count += 1
            if path.is_symlink():
                failures.append(f"Source symlink: {relative}")
                continue
            if path.suffix in BINARY_SUFFIXES or name.endswith((".tar.gz", ".pyc", ".log")):
                failures.append(f"Generated payload: {relative}")
                continue
            data = path.read_bytes()
            if b"\0" in data:
                if relative.as_posix() != "examples/common/DejaVuSans.ttf":
                    failures.append(f"Unexpected binary: {relative}")
                continue
            try:
                text = data.decode("utf-8-sig")
            except UnicodeDecodeError:
                failures.append(f"Invalid UTF-8: {relative}")
                continue
            for pattern in SECRET_PATTERNS:
                if re.search(pattern, text):
                    failures.append(f"Credential pattern: {relative}")
            if LOCAL_PATH.search(text):
                failures.append(f"Machine path: {relative}")
            if path.suffix == ".md" and re.search(r"CONTEXT\.txt|NEXT\.md|session Codex", text):
                failures.append(f"Private maintenance reference: {relative}")
    return count, failures


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", nargs="?", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--public", action="store_true")
    args = parser.parse_args()
    count, failures = audit(args.root.resolve(), args.public)
    for failure in failures:
        print(failure)
    if failures:
        raise SystemExit(1)
    print(f"Audited {count} source files: no detected credentials/private paths or unexpected payloads")


if __name__ == "__main__":
    main()
