"""Verify downloaded Actions archives and upload their payloads to a GitHub release."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import runpy
import shutil
import stat
import subprocess
import sys
import tempfile
import zipfile

from release_metadata import ROOT, PROJECT_NAME, PACKAGE_NAME, REPOSITORY, file_hash, release_version

REPO = "Robin-G0/MIG"
EXTENSIONS = (".zip", ".tar.gz", ".tgz", ".whl", ".deb")
METADATA = {"release-manifest.json", "SHA256SUMS"}
VERIFY_RUN = runpy.run_path(str(ROOT / "tools/check-release-run.py"))["verify_run"]


def streamed_hash(source):
    digest = hashlib.sha256()
    for block in iter(lambda: source.read(1024 * 1024), b""):
        digest.update(block)
    return digest.hexdigest()


def gh(*arguments):
    result = subprocess.run(["gh", *arguments], check=True, capture_output=True, text=True)
    return result.stdout.strip()


def api(endpoint, *arguments):
    return json.loads(gh("api", endpoint, *arguments))


def selected_version(value):
    version = value.removeprefix("v") if value else release_version()
    if not re.fullmatch(r"\d+\.\d+\.\d+", version):
        raise ValueError("Use a version such as 1.0.1 or v1.0.1")
    return version


def downloaded_archives(directory):
    if not directory.is_dir():
        raise ValueError(f"Not a directory: {directory}")
    candidates = sorted(path for path in directory.iterdir()
                        if path.name.startswith(PACKAGE_NAME + "-") and "candidate" in path.name)
    incomplete = [path.name for path in candidates if path.is_file() and path.suffix != ".zip"]
    if incomplete:
        raise ValueError(f"Incomplete or unexpected downloads: {', '.join(incomplete)}")
    archives = [path for path in candidates if path.is_file() and path.suffix == ".zip"]
    if not archives:
        raise ValueError("No motion-input-grid-*-candidate*.zip Actions downloads found")
    return archives


def extract_downloads(archives, destination):
    for path in archives:
        print(f"Extracting {path.name}", flush=True)
        with zipfile.ZipFile(path) as archive:
            for entry in archive.infolist():
                name = entry.filename
                if entry.is_dir():
                    continue
                if (not name or name in (".", "..") or any(char in name for char in "/\\:#")
                        or stat.S_ISLNK(entry.external_attr >> 16)):
                    raise ValueError(f"Unexpected Actions archive path: {name}")
                target = destination / name
                with archive.open(entry) as source:
                    if target.exists():
                        if streamed_hash(source) != file_hash(target):
                            raise ValueError(f"Conflicting files across downloaded archives: {name}")
                    else:
                        with target.open("wb") as output:
                            shutil.copyfileobj(source, output)


def verify_inventory(directory, version):
    manifest = json.loads((directory / "release-manifest.json").read_text(encoding="utf-8"))
    expected = {"project": PROJECT_NAME, "package": PACKAGE_NAME, "repository": REPOSITORY,
                "version": version, "tag": "v" + version}
    for key, value in expected.items():
        if manifest.get(key) != value:
            raise ValueError(f"Release manifest {key} does not match {value}")
    files = {}
    for entry in manifest["artifacts"]:
        name = entry["name"]
        if (not isinstance(name, str) or name in files or any(char in name for char in "/\\:#")
                or not name.startswith((PACKAGE_NAME + "-", PACKAGE_NAME + "_", "motion_input_grid-"))
                or not name.endswith(EXTENSIONS)
                or not re.search(rf"(?<![0-9]){re.escape(version)}(?![0-9]|\.[0-9])", name)):
            raise ValueError(f"Invalid, duplicate or stale release artifact: {name}")
        path = directory / name
        if path.stat().st_size != entry["bytes"] or file_hash(path) != entry["sha256"]:
            raise ValueError(f"Release manifest size/hash mismatch: {name}")
        files[name] = path
    if not files:
        raise ValueError("Release manifest contains no artifacts")
    allowed = set(files) | METADATA | {name + ".sha256" for name in files}
    extra = {path.name for path in directory.iterdir()} - allowed
    if extra:
        raise ValueError(f"Unlisted release files: {', '.join(sorted(extra))}")
    checksums = {}
    for line in (directory / "SHA256SUMS").read_text(encoding="utf-8").splitlines():
        digest, name = line.split("  ", 1)
        if name in checksums or name not in set(files) | {"release-manifest.json"}:
            raise ValueError(f"Unexpected or duplicate checksum entry: {name}")
        if not re.fullmatch(r"[0-9a-f]{64}", digest) or file_hash(directory / name) != digest:
            raise ValueError(f"Checksum mismatch: {name}")
        checksums[name] = digest
    if set(checksums) != set(files) | {"release-manifest.json"}:
        raise ValueError("SHA256SUMS does not cover the complete release inventory")
    subprocess.run([sys.executable, "-B", str(ROOT / "tests/package_artifact_tests.py"),
                    str(directory)], check=True)
    return [files[name] for name in sorted(files)] + [directory / name for name in sorted(METADATA)]


def verify_remote_run(tag):
    reference = api(f"repos/{REPO}/git/ref/tags/{tag}")["object"]
    while reference["type"] == "tag":
        reference = api(f"repos/{REPO}/git/tags/{reference['sha']}")["object"]
    if reference["type"] != "commit":
        raise ValueError("Release tag must reference a commit")
    pages = api(f"repos/{REPO}/actions/workflows/release-check.yml/runs?status=success&per_page=100",
                "--paginate", "--slurp")
    for page in pages:
        for run in page["workflow_runs"]:
            if run["head_sha"] == reference["sha"] and run["head_branch"] == tag:
                VERIFY_RUN(run, reference["sha"])
                print(f"Validated candidate run: {run['html_url']}", flush=True)
                return
    raise ValueError(f"No successful Prepare release candidates run found for tag {tag}")


def remote_release(tag):
    pages = api(f"repos/{REPO}/releases?per_page=100", "--paginate", "--slurp")
    return next((release for page in pages for release in page if release["tag_name"] == tag), None)


def missing_assets(tag, files, release):
    if release is None:
        return files
    existing = {asset["name"]: asset for asset in release["assets"]}
    missing = []
    for path in files:
        asset = existing.get(path.name)
        if asset is None:
            missing.append(path)
            continue
        expected = "sha256:" + file_hash(path)
        digest = asset.get("digest")
        if not digest:
            with tempfile.TemporaryDirectory(prefix="mig-existing-asset-") as temporary:
                gh("release", "download", tag, "--repo", REPO,
                   "--pattern", path.name, "--dir", temporary)
                digest = "sha256:" + file_hash(Path(temporary) / path.name)
        if asset["size"] != path.stat().st_size or digest != expected:
            raise ValueError(f"Existing release asset differs; refusing to replace: {path.name}")
        print(f"Already uploaded: {path.name}", flush=True)
    if missing and release.get("immutable"):
        raise ValueError("This release is immutable and cannot accept missing assets")
    return missing


def upload_release(tag, files):
    verify_remote_run(tag)
    release = remote_release(tag)
    missing = missing_assets(tag, files, release)
    if release is None:
        gh("release", "create", tag, "--repo", REPO, "--verify-tag", "--draft",
           "--title", f"{PROJECT_NAME} {tag}", "--generate-notes")
    for path in missing:
        print(f"Uploading {path.name}", flush=True)
        gh("release", "upload", tag, str(path), "--repo", REPO)
    release = remote_release(tag)
    if release is None or missing_assets(tag, files, release):
        raise ValueError("Release upload is incomplete; rerun the same command to resume")
    print(f"Release assets verified: {release['html_url']}")
    if release["draft"]:
        print("The release is a draft. Review its notes and publish it on GitHub when ready.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", nargs="?", type=Path, default=Path.cwd(),
                        help="Folder containing the downloaded Actions ZIPs (default: current folder)")
    parser.add_argument("--version", help="Expected version, e.g. 1.0.1; defaults to the repository version")
    parser.add_argument("--check-only", action="store_true", help="Verify locally without GitHub access")
    options = parser.parse_args()
    version = selected_version(options.version)
    archives = downloaded_archives(options.directory.resolve())
    if not options.check_only and not shutil.which("gh"):
        raise ValueError("Install GitHub CLI and run gh auth login first")
    work = ROOT / "build/release-upload"
    work.mkdir(parents=True, exist_ok=True)
    directory = Path(tempfile.mkdtemp(prefix=f"v{version}-", dir=work))
    print(f"Extracted files will remain in {directory}", flush=True)
    extract_downloads(archives, directory)
    files = verify_inventory(directory, version)
    print(f"Verified {len(files) - len(METADATA)} artifacts for v{version}", flush=True)
    if not options.check_only:
        upload_release("v" + version, files)


if __name__ == "__main__":
    try:
        main()
    except (ValueError, OSError, KeyError, subprocess.CalledProcessError, zipfile.BadZipFile) as error:
        print(f"Release upload failed: {error}", file=sys.stderr)
        if isinstance(error, subprocess.CalledProcessError) and error.stderr:
            print(error.stderr.strip(), file=sys.stderr)
        sys.exit(1)
