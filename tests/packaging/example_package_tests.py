"""Verify shared frozen dependencies and example SDK staging."""
import importlib.util
from pathlib import Path
import sys
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))


def load(name, file):
    spec = importlib.util.spec_from_file_location(name, ROOT / 'tools' / file)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


(ROOT / 'build').mkdir(exist_ok=True)
with tempfile.TemporaryDirectory(dir=ROOT / 'build', prefix='example-package-test-') as temporary:
    root = Path(temporary)
    source = root / 'source'
    source.mkdir()
    (source / 'python.dll').write_bytes(b'python')
    destination = root / 'destination'
    merger = load('examples_package', 'package-examples.py').merge_runtime
    merger(source, destination)
    merger(source, destination)
    (source / 'python.dll').write_bytes(b'incompatible')
    try:
        merger(source, destination)
    except RuntimeError:
        pass
    else:
        raise AssertionError('Conflicting runtimes cannot be silently merged')
    freezer = load('freeze_examples', 'freeze-python-examples.py')
    copies = [root / 'first.zip', root / 'second.zip']
    for year, copy in zip((2000, 2020), copies):
        with zipfile.ZipFile(copy, 'w') as output:
            output.writestr(zipfile.ZipInfo('module.pyc', (year, 1, 1, 0, 0, 0)), b'module')
        freezer.canonicalize_python_library(copy)
    assert copies[0].read_bytes() == copies[1].read_bytes()
    with zipfile.ZipFile(copies[0]) as archive:
        assert archive.read('module.pyc') == b'module'
    packager = load('sdk_staging', 'package-examples.py')
    sdk = root / 'sdk'
    (sdk / 'include').mkdir(parents=True)
    (sdk / 'include/header.hpp').write_text('header')
    (sdk / 'bin/models').mkdir(parents=True)
    (sdk / 'bin/mig-c.dll').write_bytes(b'abi')
    (sdk / 'bin/mig-controller.exe').write_bytes(b'app')
    (sdk / 'bin/models/full.task').write_bytes(b'model')
    staged = root / 'staged-sdk'
    packager.copy_example_sdk(sdk, staged)
    assert (staged / 'bin/mig-c.dll').read_bytes() == b'abi'
    assert not (staged / 'bin/mig-controller.exe').exists()
    assert not (staged / 'bin/models').exists()
print('Shared-runtime conflicts, deterministic libraries and example SDK staging passed')
