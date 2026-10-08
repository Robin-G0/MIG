"""Verify each wrist's ordered raise, and reject an isolated yellow-row pose."""
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "examples/python-tkinter/support"))
sys.path.insert(0, str(ROOT / "examples/python-tkinter"))
from example_usage import mig, demo_packet
Tracker = mig.Tracker


def actions_for(library, side=None, start_at_trigger=False):
    profile = (ROOT / "examples/common/raised-hands.json").read_text()
    actions = []
    with Tracker(library, profile) as tracker:
        for sequence in range(1, 91):
            packet = demo_packet(sequence)
            if side:
                missing = 16 if side == "left" else 15
                packet.body[missing * 8 + 3] = 0
            if start_at_trigger:
                for joint in (15, 16):
                    packet.body[joint * 8 + 1] = 0.33
            actions.extend(action for action, _ in tracker.update(packet))
    return actions


if __name__ == "__main__":
    library = sys.argv[1]
    assert actions_for(library, "left") == ["left_raise"]
    assert actions_for(library, "right") == ["right_raise"]
    assert actions_for(library) == ["left_raise", "right_raise"]
    assert actions_for(library, start_at_trigger=True) == []
    print("Demo profile: left/right wrists, ordered rows and trigger prerequisites passed")
