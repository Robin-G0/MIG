"""Create an individual browser tutorial from the prepared JavaScript examples tree."""
import argparse
import json
from pathlib import Path
import shutil
import tempfile

from package_linux import copy_tree, create_archive
from release_metadata import ROOT, build_directory, release_version, write_package_manifest, rewrite_package_guides


def assemble(source, folder, example):
    copy_tree(source / 'examples' / example, folder)
    if example != 'web':
        output = 'out' if example == 'next' else 'dist'
        copy_tree(source / 'examples' / example / output, folder / output)
    copy_tree(source / 'bindings/javascript', folder / 'dependencies/motion-input-grid')
    # Plain web already contains its runtime license folder. Framework notices
    # occupy their own subdirectory and must not replace those model/WASM notices.
    copy_tree(source / 'licenses/javascript', folder / 'licenses/javascript')
    shutil.copy2(source / 'LICENSE', folder / 'LICENSE')
    package_path = folder / 'package.json'
    if package_path.is_file():
        package = json.loads(package_path.read_text())
        package['dependencies']['motion-input-grid'] = 'file:./dependencies/motion-input-grid'
    else:
        package = {'private': True, 'type': 'module',
                   'dependencies': {'motion-input-grid': 'file:./dependencies/motion-input-grid'}}
    package_path.write_text(json.dumps(package, indent=2) + '\n')
    # Node is an explicit external prerequisite for a small, portable individual
    # archive. The combined archive also offers bundled per-platform interpreters.
    (folder / 'run.cmd').write_text('@echo off\r\nnode "%~dp0run.mjs"\r\n')
    (folder / 'run.sh').write_text(
        '#!/bin/sh\nset -eu\nfolder=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)\n'
        'exec node "$folder/run.mjs"\n', newline='\n')
    (folder / 'run.sh').chmod(0o755)
    rewrite_package_guides(folder)
    write_package_manifest(folder, example, 'browser')


def package(source, example, destination):
    name = f'motion-input-grid-{release_version()}-browser-{example}-standalone'
    with tempfile.TemporaryDirectory(prefix='browser-example-', dir=build_directory()) as temporary:
        folder = Path(temporary) / name
        assemble(source, folder, example)
        archive = destination / f'{name}.tar.gz'
        create_archive(folder, archive)
    print(archive)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--example', choices=('web', 'react', 'vue', 'next'), required=True)
    parser.add_argument('--destination', type=Path, default=ROOT / 'build/releases')
    args = parser.parse_args()
    args.destination.mkdir(parents=True, exist_ok=True)
    package(args.source, args.example, args.destination)
