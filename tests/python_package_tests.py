"""Install a generated wheel into an isolated environment and recognize real fixtures."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import venv

ROOT = Path(__file__).resolve().parents[1]


def verify(wheel):
    with tempfile.TemporaryDirectory(prefix="wheel-test-") as temporary:
        folder = Path(temporary)
        venv.create(folder / "venv", with_pip=True)
        python = folder / "venv" / ("Scripts/python.exe" if os.name == "nt" else "bin/python")
        subprocess.run([str(python), "-m", "pip", "install", "--no-index", "--no-deps",
                        str(wheel.resolve())], check=True)
        tests = folder / "tests"
        tests.mkdir()
        shutil.copytree(ROOT / "configs", folder / "configs")
        shutil.copytree(ROOT / "examples/common", folder / "examples/common")
        source = (ROOT / "tests/python_tests.py").read_text()
        source = source.replace('sys.path.insert(0, str(ROOT / "bindings/python"))', "")
        source = source.replace("run(sys.argv[1])", "run(None)")
        (tests / "installed_test.py").write_text(source)
        environment = os.environ.copy()
        for key in ("PYTHONPATH", "MIG_LIBRARY", "MIG_LIBRARY_PATH"):
            environment.pop(key, None)
        subprocess.run([str(python), "-I", str(tests / "installed_test.py")],
                       env=environment, cwd=folder, check=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("wheel", type=Path)
    verify(parser.parse_args().wheel)
