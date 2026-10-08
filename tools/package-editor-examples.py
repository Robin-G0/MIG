"""Stage complete editor tutorials using an already prepared native integration.

Called by package-integrations.py so release jobs publish matching editor examples
with the same ABI/platform, without a separate SDK installation for the user.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import tempfile
import zipfile

from package_linux import copy_tree
from release_metadata import ROOT, build_directory, release_version, write_package_manifest, rewrite_package_guides


def assemble(integration, folder, ecosystem, platform):
    source = ROOT / 'examples' / ('godot' if ecosystem.startswith('godot-') else ecosystem)
    if ecosystem.startswith('godot-'):
        source /= ecosystem.removeprefix('godot-')
    copy_tree(source, folder)
    if ecosystem == 'godot-gdscript':
        copy_tree(integration / 'addons', folder / 'addons')
        copy_tree(ROOT / 'integrations/godot/native', folder / 'dependencies/godot/native')
        copy_tree(ROOT / 'integrations/godot/addons', folder / 'dependencies/godot/addons')
    elif ecosystem == 'godot-csharp':
        shutil.copy2(ROOT / 'bindings/dotnet/MigTracker.cs', folder / 'MigTracker.cs')
        shutil.copy2(ROOT / 'bindings/dotnet/SyntheticFrames.cs', folder / 'SyntheticFrames.cs')
        binary = integration / 'addons/mig/bin'
        library = 'mig-c.dll' if platform.startswith('windows') else 'libmig-c.so.1'
        shutil.copy2(binary / library, folder / ('mig-c.dll' if platform.startswith('windows') else 'libmig-c.so'))
    elif ecosystem == 'unity':
        # One UPM package supplies scripts, managed bridge and platform-filtered
        # native plugin. No unresolved dependency on a second local UPM package.
        copy_tree(integration / 'Runtime', folder / 'Runtime')
        shutil.copy2(ROOT / 'bindings/dotnet/SyntheticFrames.cs', folder / 'SyntheticFrames.cs')
        package = json.loads((folder / 'package.json').read_text())
        package['dependencies'] = {}
        (folder / 'package.json').write_text(json.dumps(package, indent=2) + '\n')
    elif ecosystem == 'unreal':
        # This sample directly links C ABI; it does not require MIGRuntime.
        copy_tree(integration / 'ThirdParty', folder / 'ThirdParty')
    licenses = integration / ('addons/mig/licenses' if ecosystem.startswith('godot-') else 'licenses')
    copy_tree(licenses, folder / 'licenses')
    shutil.copy2(ROOT / 'LICENSE', folder / 'LICENSE')
    rewrite_package_guides(folder)
    if ecosystem == 'unity':
        # Import metadata uses the same deterministic generator as the runtime.
        import importlib.util
        spec = importlib.util.spec_from_file_location('integration_package', ROOT / 'tools/package-integrations.py')
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        module.unity_asset_metadata(folder)
    write_package_manifest(folder, ecosystem, platform)


def package(integration, ecosystem, platform, destination):
    name = f'motion-input-grid-{release_version()}-{platform}-{ecosystem}-standalone'
    with tempfile.TemporaryDirectory(prefix='editor-example-', dir=build_directory()) as temporary:
        folder = Path(temporary) / name
        assemble(integration, folder, ecosystem, platform)
        archive = destination / f'{name}.zip'
        with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED, strict_timestamps=False) as output:
            for file in sorted(folder.rglob('*')):
                if file.is_file():
                    output.write(file, f'{name}/{file.relative_to(folder).as_posix()}')
        checksum = hashlib.sha256(archive.read_bytes()).hexdigest()
        Path(f'{archive}.sha256').write_text(f'{checksum}  {archive.name}\n')
    print(archive)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--integration', type=Path, required=True)
    parser.add_argument('--ecosystem', choices=('godot-gdscript', 'godot-csharp', 'unity', 'unreal'), required=True)
    parser.add_argument('--platform', required=True)
    parser.add_argument('--destination', type=Path, default=ROOT / 'build/releases')
    args = parser.parse_args()
    args.destination.mkdir(parents=True, exist_ok=True)
    package(args.integration, args.ecosystem, args.platform, args.destination)
