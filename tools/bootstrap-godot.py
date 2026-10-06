"""Download pinned Godot 4.3 build dependencies and a matching headless test editor."""
import argparse
import hashlib
import os
import platform
from pathlib import Path
import urllib.request
import zipfile

from release_metadata import ROOT

ASSETS = {
    "godot-cpp.zip": (
        "https://github.com/godotengine/godot-cpp/archive/refs/tags/godot-4.3-stable.zip",
        "134299bc0809c0e68dee01793d613563ea1a53fc04f6f8cc4c3ba50625eb302d"),
    "godot-windows.zip": (
        "https://github.com/godotengine/godot/releases/download/4.3-stable/Godot_v4.3-stable_win64.exe.zip",
        "8f2c75b734bd956027ae3ca92c41f78b5d5a255dacc0f20e4e3c523c545ad410"),
    "godot-linux.zip": (
        "https://github.com/godotengine/godot/releases/download/4.3-stable/Godot_v4.3-stable_linux.x86_64.zip",
        "7de56444b130b10af84d19c7e0cf63cf9e9937ee4ba94364c3b7dd114253ca21"),
    "godot-linux-arm64.zip": (
        "https://github.com/godotengine/godot/releases/download/4.3-stable/Godot_v4.3-stable_linux.arm64.zip",
        "baaf0b753b86ef52e10c11cc7f65c0bc4696ec793ee052ea88360b31ef5462a2"),
}


def download(name, folder):
    url, expected = ASSETS[name]
    archive = folder / name
    if not archive.exists():
        urllib.request.urlretrieve(url, archive)
    if hashlib.sha256(archive.read_bytes()).hexdigest() != expected:
        raise ValueError(f"Godot download checksum mismatch: {archive}")
    with zipfile.ZipFile(archive) as package:
        package.extractall(folder)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--destination", type=Path, default=ROOT / "build/godot-deps")
    parser.add_argument("--editor", action="store_true")
    args = parser.parse_args()
    args.destination.mkdir(parents=True, exist_ok=True)
    download("godot-cpp.zip", args.destination)
    if args.editor:
        arm64 = platform.machine().lower() in ("aarch64", "arm64")
        name = "godot-windows.zip" if os.name == "nt" else "godot-linux-arm64.zip" if arm64 else "godot-linux.zip"
        download(name, args.destination)
        if os.name != "nt":
            executable = "Godot_v4.3-stable_linux.arm64" if arm64 else "Godot_v4.3-stable_linux.x86_64"
            (args.destination / executable).chmod(0o755)
