"""Verify release metadata agrees across the native SDK and language packages."""
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]


def document(path):
    return (ROOT / path).read_text(encoding="utf-8")


def verify_identity():
    identifier = "motion-input-grid"
    repository = "https://github.com/Robin-G0/MIG"
    for path in ("vcpkg.json", "ports/motion-input-grid/vcpkg.json", "bindings/javascript/package.json"):
        package = json.loads(document(path))
        assert package["name"] == identifier, path
    assert re.search(r'^name = "([^"]+)"', document("bindings/python/pyproject.toml"), re.M)[1] == identifier
    assert json.loads(document("bindings/javascript/package.json"))["homepage"] == repository
    assert json.loads(document("ports/motion-input-grid/vcpkg.json"))["homepage"] == repository
    assert "set(CPACK_PACKAGE_NAME motion-input-grid)" in document("cmake/MIGPackaging.cmake")
    unity = json.loads(document("integrations/unity/package.json"))
    assert unity["name"] == "com.robin-g0.motion-input-grid"
    assert unity["displayName"] == "Motion Input Grid"
    assert unity["name"] in json.loads(document("examples/unity/package.json"))["dependencies"]
    assert json.loads(document("integrations/unreal/MIG.uplugin"))["FriendlyName"] == "Motion Input Grid"
    assert 'name="Motion Input Grid"' in document("integrations/godot/addons/mig/plugin.cfg")
    lock = json.loads(document("package-lock.json"))["packages"]
    assert lock["bindings/javascript"]["name"] == identifier
    assert lock[f"node_modules/{identifier}"]["resolved"] == "bindings/javascript"
    for workspace in ("examples/react", "examples/vue", "examples/next"):
        assert identifier in json.loads(document(f"{workspace}/package.json"))["dependencies"]
        assert identifier in lock[workspace]["dependencies"]


def verify_versions():
    verify_identity()
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
    assert json.loads(document("ports/motion-input-grid/vcpkg.json"))["version"] == version
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
            assert package["dependencies"]["motion-input-grid"] == version
            assert lock[workspace]["dependencies"]["motion-input-grid"] == version
    print(f"Motion Input Grid package identity and versions agree on {version}")


if __name__ == "__main__":
    verify_versions()
