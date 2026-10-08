"""Build onedir camera viewers on their target OS; no interpreter needed to run."""
import os
import io
import zipfile
from importlib.metadata import distribution
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]


def canonicalize_python_library(path):
    """Discard ZIP build timestamps so identical stdlib modules share identical bytes."""
    output = io.BytesIO()
    with zipfile.ZipFile(path) as source, zipfile.ZipFile(output, 'w') as destination:
        for name in sorted(source.namelist()):
            info = zipfile.ZipInfo(name, (1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            destination.writestr(info, source.read(name))
    path.write_bytes(output.getvalue())


def freeze(technology, variant, platform):
    # Both viewers use Tk for profile picking. PyInstaller otherwise only warns
    # and emits an apparently successful executable that cannot open that UI.
    import tkinter
    try:
        tkinter.Tcl()
    except tkinter.TclError as error:
        raise RuntimeError("Cannot freeze viewers: Python's Tcl/Tk runtime is unavailable") from error
    output = ROOT / "build/frozen" / platform / technology / variant
    command = [
        sys.executable, "-m", "PyInstaller", "--noconfirm", "--onedir",
        "--name", variant, "--contents-directory", "viewer.runtime",
        "--distpath", str(output.parent),
        "--workpath", str(ROOT / "build/freeze-work" / platform / technology / variant),
        "--specpath", str(ROOT / "build/freeze-specs" / platform / technology),
        "--paths", str(ROOT / "examples" / technology / "support"),
        "--paths", str(ROOT / "bindings/python"),
        "--hidden-import", "mig", "--hidden-import", "tkinter",
        "--hidden-import", "PIL._tkinter_finder",
        "--add-data", f"{ROOT / 'examples' / technology / 'configuration'}{os.pathsep}configuration",
        str(ROOT / "examples" / technology / f"{variant}.py"),
    ]
    if technology == "pygame":
        # Pygame supports loading its bundled fonts directly from files. Its
        # optional legacy pkg_resources path can pull distro-specific modules
        # into a frozen build that are absent on the recipient's machine.
        command[3:3] = ["--exclude-module", "pkg_resources"]
    if os.name != "nt" and technology == "pygame":
        for name in ("egl.so.1", "client.so.0", "cursor.so.0", "server.so.0"):
            library = Path("/usr/lib/x86_64-linux-gnu") / f"libwayland-{name}"
            if not library.is_file():
                raise RuntimeError(f"Install the Wayland development runtime: {library}")
            command[3:3] = ["--add-binary", f"{library}{os.pathsep}."]
    subprocess.run(command, cwd=ROOT, check=True)
    canonicalize_python_library(output / "viewer.runtime/base_library.zip")
    return output


def collect_notices(platform):
    notices = ROOT / "build/frozen" / platform / "licenses"
    notices.mkdir(exist_ok=True)
    for package in ("pyinstaller", "pygame", "pillow"):
        metadata = distribution(package)
        for item in metadata.files or ():
            if not any(part.lower().startswith(("license", "copying", "notice"))
                       for part in item.parts):
                continue
            source = Path(metadata.locate_file(item))
            if source.is_file():
                target = notices / package / str(item).replace("../", "")
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(source.read_bytes())
    interpreter_notices = [Path(sys.base_prefix) / "LICENSE.txt"]
    if os.name == "nt":
        interpreter_notices.extend((Path(sys.base_prefix) / "tcl").glob("*/license.terms"))
    if os.name != "nt":
        interpreter_notices.extend(Path("/usr/share/doc").glob("*/copyright"))
    for source in interpreter_notices:
        if source.is_file() and (os.name == "nt" or source.parent.name.startswith(
                ("python3.10", "libpython3.10", "tk8.6", "tcl8.6", "python3-pil", "python3-pygame",
                 "libwayland"))):
            target = notices / f"{source.parent.name}-{source.name}"
            target.write_bytes(source.read_bytes())


def main():
    # Freeze the same wheels on both platforms. Distro Pygame can import
    # pkg_resources through undeclared dynamic dependencies (for example jaraco),
    # producing executables that only work on the original build machine.
    for package, required in (("pyinstaller", "6.16.0"), ("pillow", "11.3.0"), ("pygame", "2.6.1")):
        if distribution(package).version != required:
            raise RuntimeError(f"Freezing requires {package}=={required}; install the pinned release first")
    platform = "windows-x64" if os.name == "nt" else "linux-x64"
    for technology in ("python-tkinter", "pygame"):
        for variant in ("main", "profile"):
            print(freeze(technology, variant, platform))
    collect_notices(platform)


if __name__ == "__main__":
    main()
