"""Resolve installed SDKs first, then checkout or packaged example assets."""
import ctypes.util
import os
import sys
import tempfile
from pathlib import Path
from types import SimpleNamespace
from xml.sax.saxutils import escape

ROOT = (Path(sys.executable).resolve().parents[2] if getattr(sys, "frozen", False)
        else Path(__file__).resolve().parents[2])
FONT_CONFIGURATION = None


def configure_fonts():
    global FONT_CONFIGURATION
    fonts = ROOT / "runtime/fonts"
    if not sys.platform.startswith("linux") or not fonts.is_dir():
        return
    if "FONTCONFIG_FILE" in os.environ:
        return
    FONT_CONFIGURATION = tempfile.TemporaryDirectory(prefix="mig-fonts-")
    configuration = Path(FONT_CONFIGURATION.name) / "fonts.conf"
    configuration.write_text(
        '<fontconfig><dir>' + escape(str(fonts)) + '</dir><cachedir>'
        + escape(FONT_CONFIGURATION.name) + '</cachedir></fontconfig>', encoding="utf-8")
    os.environ["FONTCONFIG_FILE"] = str(configuration)


def native_library():
    if os.environ.get("MIG_LIBRARY"):
        return Path(os.environ["MIG_LIBRARY"])
    installed = ctypes.util.find_library("mig-c")
    if installed:
        return None
    if os.name == "nt":
        paths = ("runtime/mig-c.dll", "build/windows/src/c-api/Release/mig-c.dll",
                 "windows/mig-c.dll", "distribution/windows/mig-c.dll")
    else:
        paths = ("runtime/libmig-c.so.1", "build/release-linux-x64/src/c-api/libmig-c.so",
                 "build/linux-apps/src/c-api/libmig-c.so", "build/linux-native/src/c-api/libmig-c.so")
    return next((ROOT / name for name in paths if (ROOT / name).is_file()), None)


def runtime_directory():
    if os.environ.get("MIG_RUNTIME"):
        return Path(os.environ["MIG_RUNTIME"])
    paths = ("runtime", "build/windows/bin", "build/native-linux-deps",
             "distribution/windows", "distribution/linux")
    return next((ROOT / name for name in paths
                 if (ROOT / name / "models/pose_landmarker_lite.task").is_file()), None)


def options(smoke=False, profile_mode=False):
    bundled = ROOT / "runtime/lib"
    if sys.platform.startswith("linux") and bundled.is_dir():
        paths = os.environ.get("LD_LIBRARY_PATH", "").split(":")
        if str(bundled) not in paths:
            environment = dict(os.environ, LD_LIBRARY_PATH=":".join([str(bundled), *paths]))
            arguments = (sys.argv[1:] if getattr(sys, "frozen", False) else sys.argv)
            environment["PYINSTALLER_RESET_ENVIRONMENT"] = "1"
            os.execve(sys.executable, [sys.executable, *arguments], environment)
    configure_fonts()
    return SimpleNamespace(
        library=native_library(), runtime=None if smoke else runtime_directory(),
        profile=Path(__file__).with_name("raised-hands.json"),
        synthetic=smoke, smoke=smoke, camera=0,
        profile_mode=profile_mode,
    )
