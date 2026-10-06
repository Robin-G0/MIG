"""Check both Godot languages and guide links in the editor-source bundle."""
import importlib.util
from pathlib import Path
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))


def load_tool(name, filename):
    spec = importlib.util.spec_from_file_location(name, ROOT / "tools" / filename)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


check_docs = load_tool("check_docs", "check-docs.py")
package = load_tool("editor_package", "package-editor-examples.py")


def main():
    with tempfile.TemporaryDirectory(dir=ROOT / "build", prefix="godot-package-") as temporary:
        base = Path(temporary)
        sdk = base / "sdk"
        sdk.mkdir()
        (sdk / "placeholder").write_text("packaging fixture\n")
        folder = base / "godot"
        package.assemble(folder, "godot", sdk)
        for name in ("MigInput.cs", "MigProfileInput.cs", "MigRaisedHands.cs",
                     "MigTracker.cs", "SyntheticFrames.cs", "raised-hands.json"):
            assert (folder / "csharp" / name).is_file(), name
            assert not (folder / name).exists(), name
        for name in ("mig_input.gd", "mig_profile_input.gd", "mig_raised_hands.gd",
                     "mig.gdextension", "raised-hands.json", "native/CMakeLists.txt",
                     "tests/regression.gd"):
            assert (folder / "gdscript" / name).is_file(), name
        for guide in folder.glob("**/README*.md"):
            if guide.parent == folder or guide.parent.parent == folder:
                errors = check_docs.check(guide)
                assert not errors, (guide, errors)
    print("Godot editor bundle includes both languages, bridges and valid guide links.")


if __name__ == "__main__":
    main()
