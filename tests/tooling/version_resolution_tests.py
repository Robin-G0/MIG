"""Exercise release tag propagation and Git-free development fallback."""
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/lib"))
from release_metadata import release_version, synchronize_versions


def git(root, *args):
    subprocess.run(["git", "-c", f"safe.directory={root.as_posix()}", *args], cwd=root,
                   check=True, capture_output=True)


def verify():
    with tempfile.TemporaryDirectory(prefix="version-") as temporary:
        root = Path(temporary)
        files = ["cmake/MIGVersion.cmake", "VERSION", "vcpkg.json", "package-lock.json",
                 "bindings/python/pyproject.toml",
                 "bindings/javascript/package.json", "integrations/unity/package.json",
                 "ports/motion-input-grid/vcpkg.json", "examples/unity/package.json",
                 "integrations/unreal/MIG.uplugin", "examples/unreal/MigExample.uplugin",
                 "integrations/godot/addons/mig/plugin.cfg", "examples/common/sdk.cmake",
                 "examples/sdk-consumer/CMakeLists.txt", "examples/native-consumer/CMakeLists.txt",
                 "integrations/godot/native/CMakeLists.txt"]
        files += [f"examples/{name}/package.json" for name in ("react", "vue", "next")]
        for name in files:
            destination = root / name
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT / name, destination)
        assert release_version(root=root) == (ROOT / "VERSION").read_text().strip()
        git(root, "init")
        git(root, "add", ".")
        git(root, "-c", "user.name=Version fixture", "-c", "user.email=fixture@example.invalid",
            "commit", "-m", "fixture")
        git(root, "tag", "v2.3.4")
        assert release_version(root=root) == "2.3.4"
        output = subprocess.check_output(["cmake", "-P", str(root / "cmake/MIGVersion.cmake")],
                                         cwd=root, text=True)
        assert "MIG 2.3.4 (Git tag v2.3.4)" in output
        assert synchronize_versions(root) == "2.3.4"
        assert json.loads((root / "ports/motion-input-grid/vcpkg.json").read_text())["version"] == "2.3.4"
        assert json.loads((root / "bindings/javascript/package.json").read_text())["version"] == "2.3.4"
        assert 'version = "2.3.4"' in (root / "bindings/python/pyproject.toml").read_text()
        assert 'version="2.3.4"' in (root / "integrations/godot/addons/mig/plugin.cfg").read_text()
        try:
            release_version("v9.0.0", root)
        except ValueError:
            pass
        else:
            raise AssertionError("Mismatched release tag accepted")
        git(root, "-c", "user.name=Version fixture", "-c", "user.email=fixture@example.invalid",
            "commit", "-am", "advance")
        assert release_version(root=root) == "2.3.4"
        (root / "CMakeLists.txt").write_text(
            'cmake_minimum_required(VERSION 3.25)\n'
            'project(VersionFixture LANGUAGES NONE)\n'
            'include(cmake/MIGVersion.cmake)\n'
            'file(WRITE "${CMAKE_BINARY_DIR}/version.txt" "${MIG_VERSION}")\n')
        build = root / "build"
        subprocess.run(["cmake", "-S", str(root), "-B", str(build)],
                       check=True, capture_output=True)
        assert (build / "version.txt").read_text() == "2.3.4"
        (root / "VERSION").write_text("2.3.5\n")
        subprocess.run(["cmake", "--build", str(build)], check=True, capture_output=True)
        assert (build / "version.txt").read_text() == "2.3.5", "VERSION changes must trigger reconfiguration"
    print("Exact tag, metadata propagation, mismatched tag, fallback and rebuild version passed")


if __name__ == "__main__":
    verify()
