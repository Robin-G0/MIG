"""Start the viewer; study example_usage.py for MIG's lifecycle."""
import sys
from pathlib import Path

folder = Path(__file__).resolve().parent
sys.path.insert(0, str(folder))
sys.path.insert(0, str(folder / "support"))
from application import run

if __name__ == "__main__":
    run()
