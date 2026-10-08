"""Load a generated Godot add-on in a fresh demo project and run bridge regressions."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[2]


def verify(archive, executable):
    with tempfile.TemporaryDirectory(prefix="godot-artifact-") as temporary:
        folder = Path(temporary)
        project = folder / "project"
        shutil.copytree(ROOT / "examples/godot/gdscript", project,
                        ignore=shutil.ignore_patterns("native", ".godot", "bin"))
        with zipfile.ZipFile(archive) as package:
            package.extractall(project)
        (project / ".godot").mkdir()
        (project / ".godot/extension_list.cfg").write_text("res://addons/mig/mig.gdextension\n")
        environment = os.environ.copy()
        environment["APPDATA"] = str(folder / "userdata/roaming")
        environment["LOCALAPPDATA"] = str(folder / "userdata/local")
        for arguments in (("--editor", "--import"), ("--script", "res://tests/regression.gd")):
            result = subprocess.run([str(executable.resolve()), "--headless", "--path", str(project), *arguments],
                                    env=environment, text=True, capture_output=True, timeout=90)
            print(result.stdout)
            assert result.returncode == 0, result.stderr
            assert "SCRIPT ERROR" not in result.stderr, result.stderr
            assert "Failed to load" not in result.stderr, result.stderr
        print("Extracted Godot add-on: imports, recognition and lifecycle passed")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    parser.add_argument("--godot", type=Path, required=True)
    args = parser.parse_args()
    verify(args.archive, args.godot)
