"""Test C ABI and managed code extracted from a Unity or Unreal artifact."""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tarfile
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def verify(archive):
    with tempfile.TemporaryDirectory(prefix="integration-test-") as temporary:
        folder = Path(temporary)
        if zipfile.is_zipfile(archive):
            with zipfile.ZipFile(archive) as package:
                package.extractall(folder)
        else:
            with tarfile.open(archive) as package:
                package.extractall(folder, filter="data")
        libraries = list(folder.rglob("mig-c.dll" if os.name == "nt" else "libmig-c.so*"))
        library = next(
            path for path in libraries
            if not path.is_symlink()
            and (os.name == "nt" or re.fullmatch(r"libmig-c\.so(?:\.\d+)*", path.name))
        )
        subprocess.run([sys.executable, str(ROOT / "tests/python_tests.py"), str(library)], check=True)
        if "unity" in archive.name:
            project = folder / "managed"
            project.mkdir()
            shutil.copy2(ROOT / "tests/dotnet/Program.cs", project / "Program.cs")
            shutil.copy2(ROOT / "bindings/dotnet/SyntheticFrames.cs", project / "SyntheticFrames.cs")
            bridge = next(folder.rglob("MigTracker.cs"))
            shutil.copy2(bridge, project / "MigTracker.cs")
            (project / "test.csproj").write_text(
                '<Project Sdk="Microsoft.NET.Sdk"><PropertyGroup><OutputType>Exe</OutputType>'
                '<TargetFramework>net8.0</TargetFramework></PropertyGroup></Project>\n')
            environment = os.environ.copy()
            environment["DOTNET_CLI_HOME"] = str(folder / "dotnet-home")
            environment["DOTNET_SKIP_FIRST_TIME_EXPERIENCE"] = "1"
            environment["DOTNET_CLI_TELEMETRY_OPTOUT"] = "1"
            environment["DOTNET_GENERATE_ASPNET_CERTIFICATE"] = "false"
            if os.name == "nt":
                environment["PATH"] = str(library.parent) + os.pathsep + environment["PATH"]
            else:
                environment["LD_LIBRARY_PATH"] = str(library.parent)
            subprocess.run(["dotnet", "run", "--project", str(project), "--",
                            str(ROOT / "configs/default.json"),
                            str(ROOT / "examples/common/raised-hands.json")], env=environment, check=True)
        print(f"Packaged native integration passed: {archive.name}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    verify(parser.parse_args().archive)
