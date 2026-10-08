"""Create one independently runnable native example archive from a prepared examples tree."""
import argparse
import hashlib
from pathlib import Path
import shutil
import tempfile
import zipfile

from package_linux import copy_tree, create_archive
from release_metadata import build_directory, release_version, write_package_manifest

ROOT = Path(__file__).resolve().parents[1]
EXAMPLES = ('sdl2', 'sfml', 'pygame', 'python-tkinter', 'sdk-consumer', 'native-consumer')


def assemble(source, folder, example, platform):
    copy_tree(source / 'examples' / example, folder / 'examples' / example)
    copy_tree(source / 'examples/common', folder / 'examples/common')
    for name in ('licenses', 'configs', 'sdk', 'bindings', 'docs', 'integrations'):
        if (source / name).is_dir():
            copy_tree(source / name, folder / name)
    shutil.copy2(source / 'LICENSE', folder / 'LICENSE')
    if example != 'sdk-consumer':
        copy_tree(source / 'runtime', folder / 'runtime')
        # Camera demos select Lite. The lower-level inference consumer defaults
        # to Full, so retain that model there; never remove a required model.
        if example != 'native-consumer':
            full_model = folder / 'runtime/models/pose_landmarker_full.task'
            if full_model.is_file():
                full_model.unlink()
    for language in ('', '.fr'):
        guide = source / 'examples' / example / f'README{language}.md'
        content = guide.read_text(encoding='utf-8')
        content = content.replace('(../../docs/', '(docs/').replace('(../../bindings/', '(bindings/')
        content = content.replace('(../../integrations/', '(integrations/')
        command = ('main.exe' if example in ('pygame', 'python-tkinter') else f'mig-{example}.exe')
        if example in ('sdk-consumer', 'native-consumer'):
            command = 'run.cmd' if platform.startswith('windows') else 'sh run.sh'
        elif platform.startswith('linux'):
            command = './main' if example in ('pygame', 'python-tkinter') else f'./mig-{example}'
        instruction = (f'From this archive root: `cd examples/{example}`, then `{command}`. '
                       'Keep the whole extracted archive together; all sibling dependencies are included.\n\n')
        if language:
            instruction = (f'Depuis la racine de cette archive : `cd examples/{example}`, puis `{command}`. '
                           'Conservez toute l\'archive extraite : les dossiers voisins contiennent ses dépendances.\n\n')
        content = content.replace('\n\n', '\n\n' + instruction, 1)
        (folder / f'README{language}.md').write_text(content, encoding='utf-8')
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
