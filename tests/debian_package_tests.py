"""Exercise signed APT indexes and download generated Debian artifacts in isolation."""
import argparse
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def run(*arguments):
    subprocess.run(arguments, check=True)


def verify(packages):
    with tempfile.TemporaryDirectory(prefix="mig-apt-") as temporary:
        folder = Path(temporary)
        home = folder / "gpg"
        home.mkdir(mode=0o700)
        gpg = ["gpg", "--homedir", str(home), "--batch", "--pinentry-mode", "loopback"]
        try:
            run(*gpg, "--passphrase", "", "--quick-generate-key",
                "MIG package test <artifact-test@example.invalid>", "rsa2048", "sign", "1d")
            listing = subprocess.check_output([*gpg, "--with-colons", "--list-secret-keys"], text=True)
            fingerprint = next(line.split(":")[9] for line in listing.splitlines() if line.startswith("fpr:"))
            repo = folder / "repository"
            run(sys.executable, str(ROOT / "tools/build-apt-repository.py"),
                *[str(path.resolve()) for path in packages], "--destination", str(repo),
                "--signing-key", fingerprint, "--gnupg-home", str(home))
            run("gpgv", "--keyring", str(repo / "mig-archive-keyring.gpg"),
                str(repo / "dists/stable/InRelease"))
            run("gpgv", "--keyring", str(repo / "mig-archive-keyring.gpg"),
                str(repo / "dists/stable/Release.gpg"), str(repo / "dists/stable/Release"))
            assert "Valid-Until:" in (repo / "dists/stable/Release").read_text()
            for architecture in ("amd64", "arm64"):
                index = repo / f"dists/stable/main/binary-{architecture}/Packages"
                if index.exists():
                    for record in index.read_text().strip().split("\n\n"):
                        assert f"Architecture: {architecture}" in record
            sources = folder / "mig.sources"
            sources.write_text(f"Types: deb\nURIs: file:{repo}\nSuites: stable\nComponents: main\n"
                               f"Signed-By: {repo}/mig-archive-keyring.gpg\n")
            for directory in ("lists/partial", "cache/archives/partial", "downloads"):
                (folder / directory).mkdir(parents=True)
            (folder / "status").touch()
            apt = ["apt-get", "-o", f"Dir::Etc::sourcelist={sources}", "-o", "Dir::Etc::sourceparts=-",
                   "-o", f"Dir::State::lists={folder}/lists", "-o", f"Dir::State::status={folder}/status",
                   "-o", f"Dir::Cache={folder}/cache", "-o", "APT::Sandbox::User=root"]
            run(*apt, "update")
            subprocess.run([*apt, "download", "libmig-dev"], cwd=folder / "downloads", check=True)
            downloaded = next((folder / "downloads").glob("*.deb"))
            digest = hashlib.sha256(downloaded.read_bytes()).digest()
            assert any(hashlib.sha256(path.read_bytes()).digest() == digest for path in packages)
            print("Signed APT repository: both signatures, isolated update and package download passed")
        finally:
            subprocess.run(["gpgconf", "--homedir", str(home), "--kill", "all"], check=False)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("packages", type=Path, nargs="+")
    verify(parser.parse_args().packages)
