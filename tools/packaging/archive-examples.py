"""Create a portable examples TAR with executable launchers even on Windows."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "lib"))

from package_linux import create_archive
from release_metadata import file_hash, rewrite_package_guides
import json


def archive_examples(folder, destination):
    # A combined package includes only the tools/source relevant to its runtime.
    # Keep optional documentation references usable when their source is absent.
    rewrite_package_guides(folder)
    manifest_path = folder / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    manifest["sha256"] = {file.relative_to(folder).as_posix(): file_hash(file)
                          for file in sorted(folder.rglob('*'))
                          if file.is_file() and file != manifest_path}
    manifest_path.write_text(json.dumps(manifest, indent=2) + '\n', encoding="utf-8")
    create_archive(folder, destination)

if __name__ == "__main__":
    archive_examples(Path(sys.argv[1]), Path(sys.argv[2]))
