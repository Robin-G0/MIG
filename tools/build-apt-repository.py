"""Create and sign a local APT repository using an existing GPG signing key."""
import argparse
from datetime import datetime, timedelta, timezone
import gzip
from pathlib import Path
import re
import shutil
import subprocess


def output(*arguments, cwd=None, environment=None):
    return subprocess.check_output(arguments, cwd=cwd, env=environment)


def stage_packages(packages, root, component):
    pool = root / "pool" / component
    pool.mkdir(parents=True, exist_ok=True)
    architectures = set()
    for package in packages:
        architecture = output("dpkg-deb", "-f", str(package), "Architecture").decode().strip()
        if architecture not in ("amd64", "arm64"):
            raise ValueError(f"Unsupported package architecture: {architecture}")
        target = pool / package.name
        if target.exists() and target.read_bytes() != package.read_bytes():
            raise ValueError(f"Conflicting package: {target}")
        shutil.copy2(package, target)
        architectures.add(architecture)
    return sorted(architectures)


def write_indexes(root, suite, component, architectures):
    for architecture in architectures:
        folder = root / "dists" / suite / component / f"binary-{architecture}"
        folder.mkdir(parents=True, exist_ok=True)
        scanned = output("dpkg-scanpackages", "--multiversion", f"pool/{component}", cwd=root)
        records = [record for record in scanned.decode().strip().split("\n\n")
                   if f"\nArchitecture: {architecture}\n" in f"\n{record}\n"]
        data = ("\n\n".join(records) + "\n\n").encode()
        (folder / "Packages").write_bytes(data)
        (folder / "Packages.gz").write_bytes(gzip.compress(data, mtime=0))
    distribution = root / "dists" / suite
    expires = (datetime.now(timezone.utc) + timedelta(days=30)).strftime("%a, %d %b %Y %H:%M:%S UTC")
    release = output("apt-ftparchive", "-o", "APT::FTPArchive::Release::Origin=MIG",
                     "-o", "APT::FTPArchive::Release::Label=MIG",
                     "-o", f"APT::FTPArchive::Release::Suite={suite}",
                     "-o", f"APT::FTPArchive::Release::Codename={suite}",
                     "-o", f"APT::FTPArchive::Release::Components={component}",
                     "-o", f"APT::FTPArchive::Release::Architectures={' '.join(architectures)}",
                     "-o", f"APT::FTPArchive::Release::Valid-Until={expires}",
                     "release", ".", cwd=distribution)
    (distribution / "Release").write_bytes(f"Valid-Until: {expires}\n".encode() + release)
    return distribution


def sign_repository(distribution, key, home):
    command = ["gpg", "--batch", "--yes"]
    if home:
        command.extend(["--homedir", str(home)])
    command.extend(["--local-user", key])
    for name, flags in (("InRelease", ["--clearsign"]), ("Release.gpg", ["--armor", "--detach-sign"])):
        subprocess.run([*command, "--output", str(distribution / name), *flags,
                        str(distribution / "Release")], check=True)
    public_key = output(*command, "--export", key)
    if not public_key:
        raise ValueError("The signing key has no exportable public key")
    (distribution.parents[1] / "mig-archive-keyring.gpg").write_bytes(public_key)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("packages", nargs="+", type=Path)
    parser.add_argument("--destination", type=Path, required=True)
    parser.add_argument("--signing-key", required=True)
    parser.add_argument("--gnupg-home", type=Path)
    parser.add_argument("--suite", default="stable")
    parser.add_argument("--component", default="main")
    args = parser.parse_args()
    for value in (args.suite, args.component):
        if not re.fullmatch(r"[a-z][a-z0-9-]*", value):
            parser.error("Suite and component must be simple lowercase names")
    if not re.fullmatch(r"(?:[0-9a-fA-F]{40}|[0-9a-fA-F]{64})", args.signing_key):
        parser.error("Use the full signing key fingerprint")
    root = args.destination.resolve()
    distribution = root / "dists" / args.suite
    if distribution.exists():
        parser.error("Use a fresh destination; do not overwrite a published repository")
    root.mkdir(parents=True, exist_ok=True)
    architectures = stage_packages([path.resolve() for path in args.packages], root, args.component)
    sign_repository(write_indexes(root, args.suite, args.component, architectures),
                    args.signing_key, args.gnupg_home)
    print(root)


if __name__ == "__main__":
    main()
