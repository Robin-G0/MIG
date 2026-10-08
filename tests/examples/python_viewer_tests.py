import contextlib
import io
import json
import sys
import tempfile
import threading
import time
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "examples/common"))
from python_source import InputSource, Tracker, announce


def wait_for_notice(source):
    deadline = time.monotonic() + 5
    while time.monotonic() < deadline:
        if notice := source.notice():
            return notice
        time.sleep(0.01)
    raise AssertionError("Profile import did not finish")


def verify_import_and_shutdown(library):
    calls = []

    class ObservedTracker(Tracker):
        def import_json(self, profile):
            calls.append(("import", threading.get_ident()))
            return super().import_json(profile)

        def close(self):
            calls.append(("close", threading.get_ident()))
            return super().close()

    configuration = json.loads((ROOT / "examples/common/raised-hands.json").read_text())
    configuration["inputs"][0]["action"] = "custom_action"
    options = SimpleNamespace(library=library, profile=ROOT / "examples/common/raised-hands.json",
                              profile_mode=True, smoke=False, synthetic=True)
    with tempfile.TemporaryDirectory() as directory, patch("python_source.Tracker", ObservedTracker):
        valid = Path(directory) / "valid.json"
        invalid = Path(directory) / "invalid.json"
        valid.write_text(json.dumps(configuration))
        invalid.write_text("{}")
        source = InputSource(options)
        try:
            source.import_profile(valid)
            assert "imported" in wait_for_notice(source)
            source.import_profile(invalid)
            assert "Import failed" in wait_for_notice(source)
            actions = []
            deadline = time.monotonic() + 5
            while time.monotonic() < deadline and not actions:
                if latest := source.take():
                    assert latest[3] is None, latest[3]
                    actions.extend(latest[1])
                time.sleep(0.01)
            assert [action for action, _ in actions] == ["custom_action", "right_raise"], actions
        finally:
            source.close()
        assert not source.worker.is_alive()
        assert all(owner == source.worker.ident for _, owner in calls), calls
        assert calls[-1][0] == "close"


def verify_messages():
    with contextlib.redirect_stdout(io.StringIO()):
        assert announce([("left_raise", "a"), ("right_raise", "b")]) == (
            "Left hand raised! | Right hand raised!")
        assert announce([("custom_action", "a")], True) == "custom_action (input a)"


def verify_pygame_quit():
    import importlib.util
    import os
    os.environ["SDL_VIDEODRIVER"] = "dummy"
    import pygame
    spec = importlib.util.spec_from_file_location("pygame_example", ROOT / "examples/pygame/main.py")
    viewer = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewer)
    order = []
    real_quit = pygame.quit

    class Source:
        def __init__(self, options):
            pass

        def close(self):
            order.append("source closed")

    def quit_display():
        order.append("display closed")
        real_quit()

    with patch.object(viewer, "InputSource", Source), patch.object(viewer, "draw_frame") as draw:
        queued = [pygame.event.Event(pygame.QUIT),
                  pygame.event.Event(pygame.MOUSEBUTTONUP, button=1, pos=(20, 20))]
        with patch.object(pygame.event, "get", return_value=queued):
            with patch.object(pygame, "quit", quit_display):
                with patch.object(viewer, "import_profile") as importer:
                    viewer.run(SimpleNamespace(profile_mode=True, synthetic=True, smoke=False))
                    importer.assert_not_called()
        draw.assert_not_called()
    assert order == ["source closed", "display closed"], order


if __name__ == "__main__":
    verify_import_and_shutdown(sys.argv[1])
    verify_messages()
    verify_pygame_quit()
    print("Viewer profile import, atomic rejection, generic actions and owner-thread shutdown passed")
