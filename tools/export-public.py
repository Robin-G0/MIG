"""Copy the current public source into a fresh destination without Git or build state."""
import argparse
from pathlib import Path
import shutil
import subprocess

from distribution_policy import GENERATED_DIRECTORIES, GENERATED_PATHS, private_file

ROOT = Path(__file__).resolve().parents[1]


def selected_files():
    command = ["git", "-c", f"safe.directory={ROOT.as_posix()}", "ls-files",
               "-z", "--cached", "--others", "--exclude-standard"]
    result = subprocess.check_output(command, cwd=ROOT).decode("utf-8")
    for name in sorted(set(result.split("\0")) - {""}):
        path = Path(name)
        if any(part in GENERATED_DIRECTORIES or private_file(part) for part in path.parts):
            continue
        if name in GENERATED_PATHS or any(name.startswith(folder + "/") for folder in GENERATED_PATHS):
            continue
        source = ROOT / path
        if not source.is_file():
            continue
        if source.is_symlink() or not source.resolve().is_relative_to(ROOT):
            raise ValueError(f"Refusing source outside checkout: {name}")
        yield path


def export(destination, dry_run=False):
    if destination.is_symlink():
        raise ValueError("Use a real destination directory, not a symlink")
    target = destination.resolve()
    if target == ROOT or ROOT.is_relative_to(target):
        raise ValueError("Destination cannot contain the source checkout")
    if target.exists() and any(target.iterdir()):
        raise ValueError("Use a fresh empty destination; existing files are never deleted")
    files = list(selected_files())
    if not dry_run:
        target.mkdir(parents=True, exist_ok=True)
        for relative in files:
            source = ROOT / relative
            output = target / relative
            output.parent.mkdir(parents=True, exist_ok=True)
            data = source.read_bytes()
            if b"\0" not in data:
                data = data.replace(b"\r\n", b"\n")
                if source.suffix == ".cmd":
                    data = data.replace(b"\n", b"\r\n")
            output.write_bytes(data)
            shutil.copymode(source, output)
    print(f"{'Selected' if dry_run else 'Exported'} {len(files)} public files to {target}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--destination", type=Path, required=True)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()
    export(args.destination, args.dry_run)


if __name__ == "__main__":
    main()
