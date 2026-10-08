"""Write categorized download notes from a release manifest or GitHub release JSON."""
import argparse
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'lib'))
from release_notes import merge_download_notes


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('inventory', type=Path, help='Local manifest or saved GitHub release JSON')
    parser.add_argument('--output', required=True, type=Path)
    options = parser.parse_args()
    data = json.loads(options.inventory.read_text(encoding='utf-8-sig'))
    assets = data['assets'] if 'assets' in data else data['artifacts']
    names = [asset['name'] for asset in assets]
    if 'artifacts' in data:
        names += ['SHA256SUMS', 'release-manifest.json']
    body = merge_download_notes(data.get('body'), data.get('tag_name') or data['tag'], names)
    options.output.parent.mkdir(parents=True, exist_ok=True)
    options.output.write_text(body, encoding='utf-8')
    print(options.output)


if __name__ == '__main__':
    main()
