"""Run each extracted tutorial's launcher, HTTP assets and real WASM browser actions."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tarfile
import tempfile
import time
import urllib.request

ROOT = Path(__file__).resolve().parents[2]


def verify(archive):
    with tempfile.TemporaryDirectory(prefix='browser-tutorial-') as temporary:
        extracted = Path(temporary)
        with tarfile.open(archive) as package:
            package.extractall(extracted, filter='data')
        folder = next(extracted.iterdir())
        manifest = json.loads((folder / 'manifest.json').read_text())
        for name, digest in manifest['sha256'].items():
            assert hashlib.sha256((folder / name).read_bytes()).hexdigest() == digest, name
        technology = manifest['ecosystem']
        output = folder / ('out' if technology == 'next' else '.' if technology == 'web' else 'dist')
        assets = output if technology == 'web' else output / 'mig'
        # Use the documented entry, from outside the folder, without repository
        # server helpers. It must serve both variants and their local WASM/models.
        process = subprocess.Popen(['node', str(folder / 'run.mjs')], cwd=extracted,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        try:
            deadline = time.monotonic() + 15
            while True:
                if process.poll() is not None:
                    raise AssertionError(process.stderr.read().decode())
                try:
                    with urllib.request.urlopen('http://localhost:8820', timeout=1) as page:
                        assert page.status == 200
                    break
                except OSError:
                    if time.monotonic() >= deadline:
                        raise
                    time.sleep(.1)
            for filename in ('profile', 'mig/mig.wasm', 'mig/default.json') if technology == 'next' else (
                    'profile.html', 'mig.wasm' if technology == 'web' else 'mig/mig.wasm'):
                with urllib.request.urlopen('http://localhost:8820/' + filename, timeout=5) as response:
                    assert response.status == 200
        finally:
            process.terminate()
            process.communicate(timeout=10)
        assert (assets / 'models/pose_landmarker_lite.task').is_file()
        assert (assets / 'vision/vision_bundle.mjs').is_file()
        environment = dict(os.environ, MIG_EXAMPLE_DIR=str(folder), MIG_EXAMPLE_TYPE=technology)
        subprocess.run(['node', str(ROOT / 'tests/bindings/javascript/framework_browser_tests.mjs')],
                       env=environment, check=True, timeout=90)
        print(f'{technology}: standalone launcher, assets, actions, import and shutdown passed')


if __name__ == '__main__':
    for pattern in sys.argv[1:]:
        paths = sorted(Path(pattern).parent.glob(Path(pattern).name))
        if not paths:
            raise FileNotFoundError(pattern)
        for path in paths:
            verify(path)
