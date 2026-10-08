"""Check isolated editor projects/plugins and compile the packaged Godot .NET tutorial."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
import zipfile


def verify(archive, build_csharp):
    with tempfile.TemporaryDirectory(prefix='editor-tutorial-') as temporary:
        extracted = Path(temporary)
        with zipfile.ZipFile(archive) as package:
            for name in package.namelist():
                assert not Path(name).is_absolute() and '..' not in Path(name).parts
            package.extractall(extracted)
        folder = next(extracted.iterdir())
        manifest = json.loads((folder / 'manifest.json').read_text())
        for name, digest in manifest['sha256'].items():
            assert hashlib.sha256((folder / name).read_bytes()).hexdigest() == digest, name
        ecosystem = manifest['ecosystem']
        assert (folder / 'README.md').is_file() and (folder / 'README.fr.md').is_file()
        if ecosystem == 'unity':
            package = json.loads((folder / 'package.json').read_text())
            assert not package['dependencies'], 'Unresolved UPM runtime dependency'
            assert (folder / 'Runtime/Bridge/MigTracker.cs').is_file()
            assert (folder / 'SyntheticFrames.cs').is_file()
            assembly = json.loads((folder / 'MIG.Examples.asmdef').read_text())
            assert 'MIG.Runtime' in assembly['references']
            importer = (folder / 'Resources/MIG/raised-hands.json.meta').read_text()
            assert 'TextScriptImporter:' in importer, 'Resources.Load needs a TextAsset'
            assert list((folder / 'Runtime/Plugins').rglob('*.meta'))
        elif ecosystem == 'unreal':
            assert (folder / 'ThirdParty/include/mig/c/api.h').is_file()
            assert (folder / 'Content/raised-hands.json').is_file()
            assert (folder / 'MigExample.uplugin').is_file()
            assert list((folder / 'ThirdParty').rglob('mig-c.dll')) or list((folder / 'ThirdParty').rglob('libmig-c.so.1'))
        elif ecosystem in ('godot-gdscript', 'godot-csharp'):
            assert (folder / 'project.godot').is_file()
            assert (folder / 'raised_hands.tscn').is_file() and (folder / 'profile.tscn').is_file()
            assert (folder / 'raised-hands.json').is_file()
            if ecosystem == 'godot-gdscript':
                assert (folder / 'addons/mig/mig.gdextension').is_file()
            else:
                assert (folder / 'MigTracker.cs').is_file() and (folder / 'SyntheticFrames.cs').is_file()
                if build_csharp:
                    environment = dict(os.environ, DOTNET_CLI_HOME=str(extracted / 'dotnet-home'),
                                       DOTNET_SKIP_FIRST_TIME_EXPERIENCE='1', DOTNET_CLI_TELEMETRY_OPTOUT='1',
                                       DOTNET_GENERATE_ASPNET_CERTIFICATE='false')
                    subprocess.run(['dotnet', 'build', str(folder / 'MigExample.csproj'),
                                    '--configuration', 'Release', '--nologo'], cwd=extracted,
                                   env=environment, check=True, timeout=180)
        else:
            raise ValueError(f'Not an editor example: {ecosystem}')
        print(f'{ecosystem}: isolated project/plugin dependencies and metadata passed')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archives', nargs='+')
    parser.add_argument('--build-csharp', action='store_true')
    args = parser.parse_args()
    for pattern in args.archives:
        paths = sorted(Path(pattern).parent.glob(Path(pattern).name))
        if not paths:
            raise FileNotFoundError(pattern)
        for path in paths:
            verify(path, args.build_csharp)
