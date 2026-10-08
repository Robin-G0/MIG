"""Create a portable examples TAR with executable launchers even on Windows."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "lib"))
from pathlib import Path
import sys

from package_linux import create_archive

if __name__ == "__main__":
    create_archive(Path(sys.argv[1]), Path(sys.argv[2]))
