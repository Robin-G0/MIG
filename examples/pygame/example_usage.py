"""Worker-owned camera with a bounded mailbox and separate synthetic smoke fixtures."""
import argparse
import json
import queue
import sys
import threading
from pathlib import Path
from python_runtime import options

try:
    import mig
except ModuleNotFoundError as error:
    if error.name != "mig":
        raise
    bindings = Path(__file__).resolve().parent / "bindings" / "python"
    if not bindings.is_dir():
        bindings = Path(__file__).resolve().parents[2] / "bindings/python"
    if not (bindings / "mig" / "__init__.py").is_file():
        raise
    sys.path.insert(0, str(bindings))
    import mig


def parse_options(profile_mode=False):
    parser = argparse.ArgumentParser(description="MIG mirrored camera and wrist tracking")
    parser.add_argument("--smoke", action="store_true", help=argparse.SUPPRESS)
    settings = options(parser.parse_args().smoke, profile_mode)
    settings.profile = Path(__file__).resolve().parent / "configuration/raised-hands.json"
    return settings


def synthetic_packet(sequence):
    """A deterministic left-wrist rise for configs/default.json, not detection."""
    packet = mig.Packet()
    packet.sequence = sequence
    packet.timestamp_ms = sequence * 20
    packet.aspect = 1
    packet.set_body(11, 0.65, 0.45)
    packet.set_body(12, 0.35, 0.45)
    row = 5.5 if sequence < 60 else 4.5 if sequence < 70 else 2.5
    packet.set_body(15, 0.62, 0.45 + (row - 3.5) * 0.06)
    packet.set_body(16, 0.25, 0.65)
    return packet


def demo_packet(sequence):
    packet = synthetic_packet(sequence)
    row = 5.5 if sequence < 60 else max(1.5, 5.5 - (sequence - 59) / 5)
    for joint, column in ((15, 6.5), (16, 2.5)):
        packet.set_body(joint, 0.5 + (column - 4.5) * 0.06, 0.45 + (row - 3.5) * 0.06)
    return packet


def load_configuration(settings):
    """Read MIG JSON before creating the tracker; import mode starts without rules."""
    configuration = json.loads(settings.profile.read_text(encoding="utf-8"))
    if settings.profile_mode and not settings.smoke:
        configuration["inputs"] = []
        configuration["tracking"]["hands"] = False
    return json.dumps(configuration)


def initialize_mig(settings, configuration):
    # Tracker(library, profile_json) validates the rules and owns a native engine.
    # library is the bundled C ABI DLL/SO, or None for a system installation.
    return mig.Tracker(settings.library, configuration)


def start_tracking_camera(tracker, settings):
    if settings.synthetic:
        return
    if settings.runtime is None:
        raise RuntimeError("Native runtime/models missing; see README dependencies.")
    # start_camera(runtime_directory, device_index) owns capture and inference.
    tracker.start_camera(settings.runtime, settings.camera)


def process_tracking_frame(tracker, settings, sequence):
    if settings.synthetic:
        tracking_frame = demo_packet(sequence)
        # Host-provided packets carry unmirrored landmarks, sequence and time.
        detected_actions = tracker.update(tracking_frame)
    else:
        # poll_camera() already evaluates MIG rules. Do not update() it twice.
        tracking_frame = tracker.poll_camera()
        detected_actions = tracker.events()
    # Copy action tuples now: native results belong to the current update.
    return tracking_frame, detected_actions


class InputSource:
    """Own the tracker exclusively on one worker, keeping GUI callbacks responsive.

    take() returns (packet, events, image, error). Actions are accumulated across mailbox
    overwrites up to a fixed 64-item bound; images are never recorded or queued.
    """

    def __init__(self, options):
        self.options = options
        self.mailbox = queue.Queue(maxsize=1)
        self.stopping = threading.Event()
        self.imports = queue.Queue(maxsize=1)
        self.notices = queue.Queue(maxsize=1)
        self.worker = threading.Thread(target=self._run, name="MIG inference")
        self.worker.start()

    def _publish(self, packet, events, image=None, error=None):
        try:
            _, previous_events, _, _ = self.mailbox.get_nowait()
        except queue.Empty:
            previous_events = []
        self.mailbox.put_nowait((packet, (previous_events + events)[-64:], image, error))

    def _run(self):
        try:
            configuration = load_configuration(self.options)
            # The worker exclusively owns the tracker. The context manager calls
            # close() even if a camera or configuration operation raises an error.
            with initialize_mig(self.options, configuration) as tracker:
                start_tracking_camera(tracker, self.options)
                sequence = 0
                synthetic_image = (320, 240, bytes((24, 30, 40)) * (320 * 240))
                while not self.stopping.is_set():
                    self._import_pending(tracker)
                    sequence += 1
                    packet, events = process_tracking_frame(tracker, self.options, sequence)
                    image = (synthetic_image if self.options.synthetic else
                             tracker.camera_image() if packet else None)
                    self._publish(packet, events, image)
                    if self.options.synthetic:
                        self.stopping.wait(0.02)
        except Exception as error:
            self._publish(None, [], error=str(error))

    def take(self):
        try:
            return self.mailbox.get_nowait()
        except queue.Empty:
            return None

    def import_profile(self, path):
        try:
            self.imports.get_nowait()
        except queue.Empty:
            pass
        self.imports.put_nowait(Path(path))

    def _import_pending(self, tracker):
        try:
            path = self.imports.get_nowait()
        except queue.Empty:
            return
        try:
            if path.stat().st_size > 1024 * 1024:
                raise ValueError("Configuration exceeds 1 MiB")
            tracker.import_json(path.read_text(encoding="utf-8"))
            self.take()
            message = f"{path.name}: imported. Keep shoulders visible to recalibrate."
        except Exception as error:
            message = f"Import failed: {error}"
        else:
            if not self.options.synthetic:
                tracker.start_camera(self.options.runtime, self.options.camera)
        try:
            self.notices.get_nowait()
        except queue.Empty:
            pass
        self.notices.put_nowait(message)

    def notice(self):
        try:
            return self.notices.get_nowait()
        except queue.Empty:
            return None

    def close(self):
        """Stop and join before the GUI destroys its resources; worker closes MIG."""
        self.stopping.set()
        self.worker.join()


def visible_points(packet):
    """Image points mirrored exactly once for display; saved anatomy is unchanged."""
    if packet is None:
        return
    for joint in range(33):
        offset = joint * 8
        if packet.body[offset + 3] >= 0.6:
            yield 1 - packet.body[offset], packet.body[offset + 1]


def wrists(packet):
    if packet is None:
        return
    for side, joint in (("left", 15), ("right", 16)):
        offset = joint * 8
        if packet.body[offset + 3] >= 0.6:
            yield side, 1 - packet.body[offset], packet.body[offset + 1]


def hand_lines(packet):
    if packet is None:
        return
    for hand in range(packet.hand_count):
        for base in (1, 5, 9, 13, 17):
            previous = hand * 126
            for joint in range(base, base + 4):
                current = hand * 126 + joint * 6
                yield (1 - packet.hands[previous], packet.hands[previous + 1],
                       1 - packet.hands[current], packet.hands[current + 1])
                previous = current


def handle_detected_actions(events, profile_mode=False):
    """Map logical (action, input_id) tuples to application behavior, never OS keys."""
    messages = []
    for action, input_id in events:
        message = (f"{action.split('_')[0].capitalize()} hand raised!"
                   if not profile_mode and action in ("left_raise", "right_raise")
                   else f"{action} (input {input_id})")
        print(message, flush=True)
        messages.append(message)
    return " | ".join(messages) or None


# The view calls this after receiving the worker's copied logical actions.
announce = handle_detected_actions
