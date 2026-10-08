"""Run deterministic viewer workflows after extracting a single native example package."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[2]


def verify(archive):
    with tempfile.TemporaryDirectory(prefix='standalone-test-', dir=ROOT / 'build') as temporary:
        extracted = Path(temporary)
        with zipfile.ZipFile(archive) as package:
            for entry in package.namelist():
                if '..' in Path(entry).parts or Path(entry).is_absolute():
                    raise AssertionError('Invalid package member')
            package.extractall(extracted)
        folder = next(extracted.iterdir())
        manifest = json.loads((folder / 'manifest.json').read_text(encoding='utf-8'))
        for name, digest in manifest['sha256'].items():
            assert hashlib.sha256((folder / name).read_bytes()).hexdigest() == digest, name
        example = manifest['ecosystem']
        directory = folder / 'examples' / example
        environment = dict(os.environ, MIG_RUNTIME='', MIG_LIBRARY='', SDL_VIDEODRIVER='dummy')
        if example in ('pygame', 'python-tkinter'):
            commands = [[str(directory / f'{variant}.exe'), '--smoke'] for variant in ('main', 'profile')]
            assert (directory / 'viewer.runtime/base_library.zip').is_file()
            assert not (directory / 'main.runtime').exists()
        elif example in ('sdl2', 'sfml'):
            commands = [[str(directory / f'mig-{example}{suffix}.exe'), '--smoke'] for suffix in ('', '-profile')]
        elif example == 'sdk-consumer':
            commands = [[str(directory / 'mig-sdk-example.exe'), str(folder / 'configs/default.json')]]
        else:
            commands = [[str(directory / 'mig-native-example.exe'), str(folder / 'runtime')]]
        for command in commands:
            completed = subprocess.run(command, cwd=extracted, env=environment,
                                       capture_output=True, text=True, timeout=60)
            assert completed.returncode == 0, (command, completed.stdout, completed.stderr)
            if example in ('pygame', 'python-tkinter'):
                expected = ('Left hand raised!', 'Right hand raised!') if Path(command[0]).stem == 'main' else (
                    'left_raise (input left_raise)', 'right_raise (input right_raise)')
                assert all(message in completed.stdout for message in expected), completed.stdout
            elif example in ('sdl2', 'sfml'):
                assert 'actions=2' in completed.stdout, completed.stdout
            elif example == 'sdk-consumer':
                assert 'Game event: left_raise' in completed.stdout
            else:
                assert 'sequence=1' in completed.stdout
        print(f'{example}: extracted package, both variants where present, actions and shutdown passed')


if __name__ == '__main__':
    for argument in sys.argv[1:]:
        paths = sorted(Path(argument).parent.glob(Path(argument).name))
        if not paths:
            raise FileNotFoundError(argument)
        for path in paths:
            verify(path)
