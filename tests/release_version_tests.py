"""Verify release metadata agrees across the native SDK and language packages."""
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def document(path):
    return (ROOT / path).read_text(encoding="utf-8")


def verify_versions():
    version = document("VERSION").strip()
    assert re.fullmatch(r"\d+\.\d+\.\d+", version)
    assert "VERSION ${MIG_VERSION}" in document("CMakeLists.txt")
    assert json.loads(document("vcpkg.json"))["version-string"] == version
    assert re.search(r'^version = "([^"]+)"', document("bindings/python/pyproject.toml"),
                     re.M)[1] == version
    assert "return MIG_LIBRARY_VERSION" in document("src/core/src/library.cpp")
    assert json.loads(document("examples/unity/package.json"))["version"] == version
    assert json.loads(document("examples/unreal/MigExample.uplugin"))["VersionName"] == version
    assert json.loads(document("integrations/unity/package.json"))["version"] == version
    assert json.loads(document("integrations/unreal/MIG.uplugin"))["VersionName"] == version
    assert json.loads(document("ports/mig/vcpkg.json"))["version"] == version
    assert f'version="{version}"' in document("integrations/godot/addons/mig/plugin.cfg")
    for selector in ("examples/common/sdk.cmake", "examples/sdk-consumer/CMakeLists.txt",
                     "examples/native-consumer/CMakeLists.txt"):
        requested = re.search(r"find_package\(MIG ([\d.]+)", document(selector))[1]
        assert requested == version.rsplit(".", 1)[0], selector
    lock = json.loads(document("package-lock.json"))["packages"]
    for workspace in ("bindings/javascript", "examples/react", "examples/vue", "examples/next"):
        package = json.loads(document(f"{workspace}/package.json"))
        if workspace == "bindings/javascript":
            assert package["version"] == lock[workspace]["version"] == version
        else:
            assert package["dependencies"]["@mig-input/browser"] == version
            assert lock[workspace]["dependencies"]["@mig-input/browser"] == version
    print(f"Native, Python, npm and editor metadata agree on {version}")


if __name__ == "__main__":
    verify_versions()
