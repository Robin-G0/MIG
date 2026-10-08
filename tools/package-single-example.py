"""Create one independently runnable native example archive from a prepared examples tree."""
import argparse
import hashlib
from pathlib import Path
import shutil
import tempfile
import zipfile

from package_linux import copy_tree, create_archive
from release_metadata import build_directory, release_version, write_package_manifest
from release_metadata import rewrite_package_guides

ROOT = Path(__file__).resolve().parents[1]
EXAMPLES = ('sdl2', 'sfml', 'pygame', 'python-tkinter', 'sdk-consumer', 'native-consumer')


def assemble(source, folder, example, platform):
    copy_tree(source / 'examples' / example, folder)
    shared = ['licenses']
    if example in ('sdk-consumer', 'native-consumer'):
        shared.append('configs')
    if example not in ('pygame', 'python-tkinter'):
        shared.append('sdk')
    for name in shared:
        if (source / name).is_dir():
            copy_tree(source / name, folder / name)
    if example in ('pygame', 'python-tkinter'):
        copy_tree(source / 'bindings/python', folder / 'bindings/python')
    shutil.copy2(source / 'LICENSE', folder / 'LICENSE')
    if example != 'sdk-consumer':
        copy_tree(source / 'runtime', folder / 'runtime')
        # Camera demos select Lite. The lower-level inference consumer defaults
        # to Full, so retain that model there; never remove a required model.
        if example != 'native-consumer':
            full_model = folder / 'runtime/models/pose_landmarker_full.task'
            if full_model.is_file():
                full_model.unlink()
    # Launchers resolve resources from their own location, independent of cwd.
    for name in ('run.cmd', 'run.sh', f'mig-{example}', f'mig-{example}-profile'):
        launcher = folder / name
        if launcher.is_file():
            content = launcher.read_text(encoding='utf-8')
            content = content.replace('../../runtime', 'runtime').replace('../../configs', 'configs')
            launcher.write_text(content, encoding='utf-8', newline='\n')
    rewrite_package_guides(folder)
    write_package_manifest(folder, example, platform)


def package(source, example, platform):
    releases = ROOT / 'build/releases'
    releases.mkdir(parents=True, exist_ok=True)
    name = f'motion-input-grid-{release_version()}-{platform}-{example}-standalone'
    with tempfile.TemporaryDirectory(prefix='single-example-', dir=build_directory()) as temporary:
        folder = Path(temporary) / name
        assemble(source, folder, example, platform)
        if platform.startswith('windows'):
            archive = releases / f'{name}.zip'
            with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, strict_timestamps=False) as target:
                for file in sorted(folder.rglob('*')):
                    if file.is_file():
                        target.write(file, f'{name}/{file.relative_to(folder).as_posix()}')
            checksum = hashlib.sha256(archive.read_bytes()).hexdigest()
            Path(f'{archive}.sha256').write_text(f'{checksum}  {archive.name}\n')
        else:
            archive = releases / f'{name}.tar.gz'
            create_archive(folder, archive)
    print(archive)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--platform', choices=('windows-x64', 'linux-x64'), required=True)
    parser.add_argument('--example', choices=EXAMPLES, required=True)
    parser.add_argument('--source', type=Path)
    args = parser.parse_args()
    source = args.source or ROOT / 'build/examples' / args.platform
    package(source.resolve(), args.example, args.platform)
