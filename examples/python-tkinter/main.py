import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "common"))
from tk_view import run

if __name__ == "__main__":
    run()
