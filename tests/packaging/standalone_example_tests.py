"""Run deterministic viewer workflows after extracting a single native example package."""
import hashlib
import argparse
import json
import os
from pathlib import Path
import subprocess
import shutil
import sys
import tempfile
import tarfile
import zipfile

def verify(archive, source_python=None):
    with tempfile.TemporaryDirectory(prefix='standalone-test-') as temporary:
        extracted = Path(temporary)
        package = zipfile.ZipFile(archive) if zipfile.is_zipfile(archive) else tarfile.open(archive)
        with package:
            entries = package.namelist() if isinstance(package, zipfile.ZipFile) else package.getnames()
            for entry in entries:
                if '..' in Path(entry).parts or Path(entry).is_absolute():
                    raise AssertionError('Invalid package member')
            if isinstance(package, zipfile.ZipFile):
                package.extractall(extracted)
            else:
                package.extractall(extracted, filter='data')
        folder = next(extracted.iterdir())
        manifest = json.loads((folder / 'manifest.json').read_text(encoding='utf-8'))
        for name, digest in manifest['sha256'].items():
            assert hashlib.sha256((folder / name).read_bytes()).hexdigest() == digest, name
        example = manifest['ecosystem']
        directory = folder
        extension = '.exe' if os.name == 'nt' else ''
        environment = dict(os.environ, MIG_RUNTIME='', MIG_LIBRARY='', SDL_VIDEODRIVER='dummy')
        if example in ('pygame', 'python-tkinter'):
            commands = [[str(directory / f'{variant}{extension}'), '--smoke'] for variant in ('main', 'profile')]
            if source_python:
                commands.extend([[str(source_python), str(directory / f'{variant}.py'), '--smoke']
                                 for variant in ('main', 'profile')])
            assert (directory / 'viewer.runtime/base_library.zip').is_file()
            assert not (directory / 'main.runtime').exists()
        elif example in ('sdl2', 'sfml'):
            commands = [[str(directory / f'mig-{example}{suffix}{extension}'), '--smoke'] for suffix in ('', '-profile')]
        elif example == 'sdk-consumer':
            commands = [[str(directory / f'mig-sdk-example{extension}'), str(folder / 'configuration/default.json')]]
        else:
            commands = [[str(directory / f'mig-native-example{extension}'), str(folder / 'runtime')]]
        environment['LD_LIBRARY_PATH'] = str(folder / 'runtime/lib')
        for command, working_directory in ((command, cwd) for command in commands for cwd in (folder, extracted)):
            completed = subprocess.run(command, cwd=working_directory, env=environment,
                                       capture_output=True, text=True, timeout=60)
            assert completed.returncode == 0, (command, completed.stdout, completed.stderr)
            if example in ('pygame', 'python-tkinter'):
                entry = command[1] if source_python and command[0] == str(source_python) else command[0]
                expected = ('Left hand raised!', 'Right hand raised!') if Path(entry).stem == 'main' else (
                    'left_raise (input left_raise)', 'right_raise (input right_raise)')
                assert all(message in completed.stdout for message in expected), completed.stdout
            elif example in ('sdl2', 'sfml'):
                if 'actions=2' not in completed.stdout:
                    # SFML's actual window loop reports each action; its
                    # display-free branch reports a summary instead.
                    expected = ('left_raise (input left_raise)', 'right_raise (input right_raise)') if (
                        '-profile' in Path(command[0]).name) else ('Left hand raised!', 'Right hand raised!')
                    assert all(completed.stdout.count(message) == 1 for message in expected), completed.stdout
            elif example == 'sdk-consumer':
                assert 'Game event: left_raise' in completed.stdout
            else:
                assert 'sequence=1' in completed.stdout
        print(f'{example}: extracted package, both variants where present, actions and shutdown passed')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archives', nargs='+')
    parser.add_argument('--source-python', type=lambda value: Path(shutil.which(value) or value).resolve())
    options = parser.parse_args()
    for argument in options.archives:
        paths = sorted(Path(argument).parent.glob(Path(argument).name))
        if not paths:
            raise FileNotFoundError(argument)
        for path in paths:
            verify(path, options.source_python)
