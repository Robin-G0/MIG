"""Extract downloaded Debian SDK packages and compile actual external consumers."""
import argparse
from pathlib import Path
import subprocess
import tempfile
from installed_sdk_consumer import verify_consumer

ROOT = Path(__file__).resolve().parents[2]


def verify(packages):
    with tempfile.TemporaryDirectory(prefix="mig-debian-") as temporary:
        folder = Path(temporary)
        version = (ROOT / 'VERSION').read_text().strip()
        for index, package in enumerate(packages):
            fields = {field: subprocess.check_output(
                ['dpkg-deb', '--field', str(package.resolve()), field], text=True).strip()
                for field in ('Package', 'Version', 'Architecture')}
            assert fields['Package'] == 'motion-input-grid'
            assert fields['Version'] == version
            assert fields['Architecture'] in ('amd64', 'arm64')
            extracted = folder / str(index)
            extracted.mkdir()
            subprocess.run(['dpkg-deb', '--extract', str(package.resolve()), str(extracted)], check=True)
            sdk = extracted / 'usr'
            assert (sdk / 'share/doc/motion-input-grid/copyright').is_file()
            verify_consumer(sdk, extracted, fields['Architecture'] == 'arm64')
            print(f'Debian SDK metadata, extraction and external consumer passed: {package.name}')


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("packages", type=Path, nargs="+")
    verify(parser.parse_args().packages)
