"""Command-line entry point for Linux release packaging."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "lib"))
from package_linux import main

if __name__ == "__main__":
    main()
