"""Build user-facing download categories with stable release-note headings."""
import re
from urllib.parse import quote

from release_metadata import REPOSITORY

START = '<!-- MIG-DOWNLOADS:START -->'
END = '<!-- MIG-DOWNLOADS:END -->'
ANCHOR_PREFIX = 'user-content-'
NATIVE = (
    ('python-tkinter', 'Python Tkinter examples'), ('pygame', 'Pygame examples'),
    ('sdl2', 'SDL2 examples'), ('sfml', 'SFML examples'),
    ('sdk-consumer', 'SDK consumer examples'), ('native-consumer', 'Native consumer examples'),
)


def category(name):
    if 'javascript-examples' in name or '-browser-' in name:
        return 'Browser examples'
    if any(f'-{engine}' in name for engine in ('godot', 'unity', 'unreal')):
        return 'Game engines'
    if '-standalone.' in name or '-examples.' in name:
        return 'Native examples'
    if '-native.' in name:
        return 'Desktop applications'
    if ('-sdk.' in name or name.endswith(('.whl', '.deb')) or
            name.startswith('motion_input_grid-') or
            re.fullmatch(r'motion-input-grid-[\d.]+\.tgz', name)):
        return 'SDKs and language packages'
    return 'Source and integrity files'


def platform(name):
    match = re.search(r'-(windows|linux)-(x64|arm64)-', name)
    if match:
        return f'{match[1].title()} {"ARM64" if match[2] == "arm64" else "x64"}'
    for suffix, label in (('win_amd64.whl', 'Windows x64'),
                          ('x86_64.whl', 'Linux x64'), ('aarch64.whl', 'Linux ARM64'),
                          ('_amd64.deb', 'Debian/Ubuntu x64'), ('_arm64.deb', 'Debian/Ubuntu ARM64')):
        if name.endswith(suffix):
            return label
    if '-browser-' in name or 'javascript-examples' in name:
        return 'Browser / Windows / Linux'
    if re.fullmatch(r'motion-input-grid-[\d.]+\.tgz', name):
        return 'Browser / npm'
    if name.startswith('motion_input_grid-'):
        return 'Python source'
    return 'All platforms'


def download_notes(tag, names):
    if not re.fullmatch(r'v\d+\.\d+\.\d+', tag):
        raise ValueError('Release tag must be vMAJOR.MINOR.PATCH')
    names = sorted(set(names))
    if any(not re.fullmatch(r'[A-Za-z0-9_.-]+', name) or name in ('.', '..') for name in names):
        raise ValueError('Release asset names must be plain filenames')
    groups = {title: [] for title in ('Desktop applications', 'Native examples',
                                     'Browser examples', 'Game engines',
                                     'SDKs and language packages', 'Source and integrity files')}
    for name in names:
        groups[category(name)].append(name)
    guide = f'{REPOSITORY}/blob/{tag}/readme.md'
    navigation = ' · '.join(f'[{title}](#{ANCHOR_PREFIX}{title.lower().replace(" ", "-")})'
                            for title in groups)
    # Release descriptions do not generate heading anchors. GitHub sanitizes
    # explicit HTML anchor names with its user-content- prefix.
    lines = [START, '<a name="downloads"></a>', '', '## Downloads', '',
             f'Choose your platform below. Extract archives completely and keep their files together. '
             f'See the [release README]({guide}) for requirements and installation.', '',
             navigation, '']

    def table(files):
        if not files:
            lines.extend(['No packages in this category for this release.', ''])
            return
        lines.extend(['| Platform | Download |', '| --- | --- |'])
        for name in files:
            url = f'{REPOSITORY}/releases/download/{tag}/{quote(name)}'
            lines.append(f'| {platform(name)} | [{name}]({url}) |')
        lines.append('')

    descriptions = {
        'Desktop applications': 'Configurator and controller with the camera runtime and models.',
        'Native examples': 'Camera viewers and C++ consumers with sources and runtime dependencies. '
                           'Individual archives contain one tutorial; combined archives share dependencies.',
        'Browser examples': 'Compiled browser, React, Vue and Next.js tutorials with local assets. '
                            'The combined archive bundles Node; individual archives require Node 22.12+.',
        'Game engines': 'Preview Godot, Unity and Unreal bridges and tutorial projects. '
                        'Install the matching editor and supply tracking from your application.',
        'SDKs and language packages': 'C++ SDKs, Python wheels, npm and Debian packages. '
                                    'Native SDKs and Python wheels recognize host-supplied positions; '
                                    'the npm package includes browser tracking assets.',
        'Source and integrity files': 'Source archives, vcpkg overlay and release integrity metadata. '
                                      'Verify downloads against SHA256SUMS; hashes detect corruption.',
    }
    for title, files in groups.items():
        anchor = title.lower().replace(' ', '-')
        lines.extend([f'<a name="{anchor}"></a>', '', f'## {title}', '', descriptions[title], ''])
        table(files)
        if title == 'Native examples':
            combined = [name for name in files if '-examples.' in name]
            for technology, heading in NATIVE:
                individual = [name for name in files if f'-{technology}-standalone.' in name]
                anchor = heading.lower().replace(' ', '-')
                lines.extend([f'<a name="{anchor}"></a>', '', f'### {heading}', ''])
                fallback = combined if technology not in ('sdk-consumer', 'native-consumer') else []
                if not individual and fallback:
                    lines.extend([f'Included in the combined archive under `examples/{technology}/`. '
                                  'Keep the complete archive together.', ''])
                table(individual or fallback)
    return '\n'.join(lines + [END, ''])


def merge_download_notes(body, tag, names):
    """Replace only our download block, preserving release highlights and changelogs."""
    block = download_notes(tag, names)
    body = body or ''
    if START in body or END in body:
        if body.count(START) != 1 or body.count(END) != 1 or body.index(END) < body.index(START):
            raise ValueError('Malformed release download block')
        before, remaining = body.split(START, 1)
        _, after = remaining.split(END, 1)
        return before + block.rstrip('\n') + after
    return block + ('\n' + body if body else '')
