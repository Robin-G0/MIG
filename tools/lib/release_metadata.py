"""Shared version and manifest helpers for local release builders."""
import hashlib
import json
from pathlib import Path
import re
import subprocess
import shutil

ROOT = Path(__file__).resolve().parents[2]
PROJECT_NAME = "Motion Input Grid"
PACKAGE_NAME = "motion-input-grid"
REPOSITORY = "https://github.com/Robin-G0/MIG"


def build_directory():
    directory = ROOT / "build"
    directory.mkdir(parents=True, exist_ok=True)
    return directory


def release_version(tag=None, root=ROOT):
    version = (root / "VERSION").read_text().strip()
    if (root / ".git").exists() and shutil.which("git"):
        result = subprocess.run(["git", "-c", f"safe.directory={root.as_posix()}", "describe",
                                 "--tags", "--exact-match", "HEAD"], cwd=root,
                                capture_output=True, text=True, check=False)
        exact = result.stdout.strip()
        if result.returncode == 0 and re.fullmatch(r"v\d+\.\d+\.\d+", exact):
            version = exact[1:]
    if not re.fullmatch(r"\d+\.\d+\.\d+", version):
        raise ValueError("VERSION must contain major.minor.patch")
    if tag is not None and tag != f"v{version}":
        raise ValueError(f"Tag {tag} does not match resolved version {version}")
    return version


def synchronize_versions(root=ROOT):
    version = release_version(root=root)
    (root / "VERSION").write_text(version + "\n")
    for relative, key in (
        ("vcpkg.json", "version-string"), ("bindings/javascript/package.json", "version"),
        ("integrations/unity/package.json", "version"), ("ports/motion-input-grid/vcpkg.json", "version"),
        ("examples/unity/package.json", "version"),
        ("integrations/unreal/MIG.uplugin", "VersionName"),
        ("examples/unreal/MigExample.uplugin", "VersionName"),
    ):
        path = root / relative
        content = path.read_text()
        content = re.sub(rf'("{key}"\s*:\s*")[^"]+',
                         lambda match: match[1] + version, content)
        path.write_text(content)
    path = root / "bindings/python/pyproject.toml"
    content = re.sub(r'^version = "[^"]+"', f'version = "{version}"', path.read_text(), flags=re.M)
    path.write_text(content)
    path = root / "integrations/godot/addons/mig/plugin.cfg"
    path.write_text(re.sub(r'^version="[^"]+"', f'version="{version}"', path.read_text(), flags=re.M))
    path = root / "examples/unity/package.json"
    package = json.loads(path.read_text())
    package["dependencies"]["com.robin-g0.motion-input-grid"] = version
    path.write_text(json.dumps(package, indent=4) + "\n")
    lock = root / "package-lock.json"
    data = json.loads(lock.read_text())
    data["packages"]["bindings/javascript"]["version"] = version
    for workspace in ("examples/react", "examples/vue", "examples/next"):
        path = root / workspace / "package.json"
        package = json.loads(path.read_text())
        package["dependencies"]["motion-input-grid"] = version
        path.write_text(json.dumps(package, indent=4) + "\n")
        data["packages"][workspace]["dependencies"]["motion-input-grid"] = version
    lock.write_text(json.dumps(data, indent=2) + "\n")
    major_minor = version.rsplit(".", 1)[0]
    for path in (root / "examples").rglob("*.cmake"):
        path.write_text(re.sub(r'find_package\(MIG [\d.]+', f'find_package(MIG {major_minor}', path.read_text()))
    for relative in ("examples/sdk-consumer/CMakeLists.txt", "examples/native-consumer/CMakeLists.txt",
                     "integrations/godot/native/CMakeLists.txt"):
        path = root / relative
        path.write_text(re.sub(r'find_package\(MIG [\d.]+', f'find_package(MIG {major_minor}', path.read_text()))
    return version


def file_hash(path, algorithm="sha256"):
    digest = hashlib.new(algorithm)
    with Path(path).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def write_package_manifest(folder, ecosystem, platform):
    files = {path.relative_to(folder).as_posix(): file_hash(path)
             for path in sorted(folder.rglob("*")) if path.is_file() and path.name != "manifest.json"}
    (folder / "manifest.json").write_text(json.dumps({
        "project": PROJECT_NAME, "package": PACKAGE_NAME, "repository": REPOSITORY,
        "version": release_version(), "ecosystem": ecosystem, "platform": platform,
        "sha256": files,
    }, indent=2) + "\n")


def rewrite_package_guides(folder):
    prefix = f"https://github.com/Robin-G0/MIG/blob/v{release_version()}/"
    package_root = folder.resolve()
    for guide in folder.rglob("*.md"):
        if 'licenses' in guide.relative_to(folder).parts:
            continue
        content = guide.read_text(encoding="utf-8")
        content = re.sub(r"\]\((?:\.\./)+(docs|examples)/",
                         lambda match: f"]({prefix}{match[1]}/", content)
        def missing_reference(match):
            target = match[1]
            if ':' in target or target.startswith('#'):
                return match[0]
            filename, separator, anchor = target.partition('#')
            candidate = (guide.parent / filename).resolve()
            if candidate.exists():
                return match[0]
            try:
                relative = candidate.relative_to(package_root)
            except ValueError:
                return match[0]
            # Optional API/source references can point to the matching release;
            # runtime dependencies must already exist and are tested separately.
            if (ROOT / relative).is_file():
                return f"]({prefix}{relative.as_posix()}{separator}{anchor})"
            return match[0]
        content = re.sub(r'\]\(([^)\s]+)\)', missing_reference, content)
        guide.write_text(content, encoding="utf-8")
