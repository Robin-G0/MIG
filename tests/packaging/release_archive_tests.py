"""Check archive manifests, SDK symlinks and included documentation/licenses."""
import hashlib
import json
from pathlib import Path
import sys
import tarfile
import zipfile


def check_example_contents(path, names, root, extension):
    required = ["README.fr.md", "docs/getting-started/bootstrap.md", "docs/getting-started/bootstrap.fr.md",
                "docs/getting-started/examples.md", "docs/getting-started/examples.fr.md"]
    if "-javascript-examples." in path.name:
        for technology in ("web", "react", "vue", "next"):
            required.extend(f"examples/{technology}/{file}"
                            for file in ("run.cmd", "run.sh", "README.md", "README.fr.md"))
        required.extend(f"runtime/node/{platform}/{binary}" for platform, binary in (
            ("win-x64", "node.exe"), ("linux-x64", "node"), ("linux-arm64", "node")))
        required.append("bindings/javascript/runtime/vision/vision_bundle.mjs")
    else:
        for technology in ("python-tkinter", "pygame"):
            for variant in ("main", "profile"):
                required.extend((f"examples/{technology}/{variant}{extension}",
                                 f"examples/{technology}/{variant}.py"))
    for name in required:
        assert f"{root}/{name}" in names, name


def check_tar(path):
    with tarfile.open(path) as archive:
        entries = {item.name: item for item in archive.getmembers()}
        manifest_name = next(name for name in entries if name.endswith("/manifest.json"))
        root = manifest_name.removesuffix("/manifest.json")
        manifest = json.load(archive.extractfile(manifest_name))
        for name, expected in manifest["sha256"].items():
            content = archive.extractfile(f"{root}/{name}").read()
            assert hashlib.sha256(content).hexdigest() == expected, name
        standalone = '-standalone.' in path.name
        required = ("LICENSE", "README.md", "README.fr.md") if "-examples." in path.name or standalone else (
            "LICENSE", "README.md", "docs/reference/support.md", "docs/getting-started/packages.md")
        for name in required:
            assert f"{root}/{name}" in entries, name
        if standalone:
            assert manifest['ecosystem']
            assert not any('/examples/' in name for name in entries), 'Individual example is not its package root'
        elif "-examples." not in path.name:
            assert any(item.issym() and item.name.endswith("libmig-c.so.1")
                       for item in entries.values()), "SDK SONAME link missing"
        else:
            check_example_contents(path, entries, root, "")
            if "-linux-x64-examples." in path.name:
                for technology in ("python-tkinter", "pygame"):
                    for variant in ("main", "profile"):
                        executable = entries[f"{root}/examples/{technology}/{variant}"]
                        assert executable.mode & 0o111, executable.name
        for item in entries.values():
            if item.isfile() and (item.name.endswith("/node") or item.name.endswith(".sh")):
                assert item.mode & 0o111, f"Executable permissions missing: {item.name}"


def check_zip(path):
    with zipfile.ZipFile(path) as archive:
        manifest_name = next(name for name in archive.namelist()
                             if name == "manifest.json" or name.endswith("/manifest.json"))
        prefix = manifest_name.removesuffix("manifest.json")
        root = prefix.rstrip('/')
        manifest = json.loads(archive.read(manifest_name).decode("utf-8-sig"))
        for name, expected in manifest["sha256"].items():
            assert hashlib.sha256(archive.read(f"{prefix}{name}")).hexdigest() == expected, name
        guide_folder = prefix + ('addons/mig/' if manifest.get('ecosystem') == 'godot' else '')
        assert archive.read(f"{guide_folder}LICENSE")
        assert archive.read(f"{guide_folder}README.fr.md")
        if '-standalone.' in path.name:
            assert manifest['ecosystem']
            assert not any('/examples/' in name for name in archive.namelist())
        if "-examples." in path.name:
            check_example_contents(path, archive.namelist(), root, ".exe")


def main():
    directory = Path(sys.argv[1] if len(sys.argv) > 1 else "build/releases")
    for path in sorted(directory.glob("motion-input-grid-*.tar.gz")):
        check_tar(path)
        print(f"Verified {path.name}")
    for path in sorted(directory.glob("motion-input-grid-*.zip")):
        check_zip(path)
        print(f"Verified {path.name}")


if __name__ == "__main__":
    main()
