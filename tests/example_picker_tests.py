import os
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def find_window(name, process, visible=True):
    deadline = time.monotonic() + 5
    while time.monotonic() < deadline and process.poll() is None:
        command = ["xdotool", "search"]
        if visible:
            command.append("--onlyvisible")
        result = subprocess.run([*command, "--name", name],
                                capture_output=True, text=True)
        if result.returncode == 0:
            return result.stdout.splitlines()[-1]
        time.sleep(0.02)
    raise RuntimeError(f"Missing window: {name}; process exit: {process.poll()}")


def focus_window(window, process):
    deadline = time.monotonic() + 5
    while time.monotonic() < deadline and process.poll() is None:
        try:
            result = subprocess.run(["xdotool", "windowfocus", "--sync", window],
                                    capture_output=True, text=True, timeout=1)
            if result.returncode == 0:
                return
        except subprocess.TimeoutExpired:
            pass
        time.sleep(0.02)
    raise RuntimeError(f"Cannot focus window: {window}; process exit: {process.poll()}")


def wait_ready(log, process):
    deadline = time.monotonic() + 5
    while time.monotonic() < deadline and process.poll() is None:
        if "Keep shoulders visible." in log.read_text(encoding="utf-8", errors="replace"):
            return
        time.sleep(0.02)
    raise RuntimeError(f"Example did not become ready; process exit: {process.poll()}")


def verify_picker(binary, profile):
    log = ROOT / "build/example-picker.log"
    log.parent.mkdir(parents=True, exist_ok=True)
    with log.open("w") as output:
        environment = dict(os.environ, QT_QPA_PLATFORM="xcb")
        process = subprocess.Popen([str(Path(binary).resolve()), "--smoke"], env=environment,
                                   stdout=output, stderr=subprocess.STDOUT)
        try:
            wait_ready(log, process)
            host = find_window("^MIG SFML", process, visible=False)
            subprocess.run(["xdotool", "windowmap", "--sync", host], check=True, timeout=5)
            focus_window(host, process)
            subprocess.run(["xdotool", "mousemove", "--sync", "--window", host,
                            "50", "30", "click", "1"],
                           check=True)
            dialog = find_window("Import MIG profile", process)
            focus_window(dialog, process)
            subprocess.run(["xdotool", "key", "ctrl+l"], check=True)
            subprocess.run(["xdotool", "type", "--clearmodifiers", str(profile.resolve())], check=True)
            subprocess.run(["xdotool", "key", "Return"], check=True)
            result = process.wait(timeout=8)
            assert result == 0, f"Exit code: {result}\n{log.read_text()}"
            print("Qt profile picker import and shutdown passed")
        finally:
            if process.poll() is None:
                process.terminate()
                process.wait(timeout=5)


if __name__ == "__main__":
    profile = Path(sys.argv[2]) if len(sys.argv) > 2 else ROOT / "examples/common/raised-hands.json"
    verify_picker(sys.argv[1], profile)
