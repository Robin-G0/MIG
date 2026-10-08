"""Exercise downloaded release validation and uploads without accessing GitHub."""
import importlib.util
import io
import json
import re
from pathlib import Path
import sys
import tarfile
import tempfile
import unittest
from unittest.mock import patch
import zipfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/lib"))
spec = importlib.util.spec_from_file_location("release_upload", ROOT / "tools/release/upload-release.py")
upload = importlib.util.module_from_spec(spec)
spec.loader.exec_module(upload)


def fixture(directory):
    artifact = directory / "motion-input-grid-1.0.1-source.tar.gz"
    with tarfile.open(artifact, "w:gz") as archive:
        license_text = b"Fixture license\n"
        entry = tarfile.TarInfo("source/LICENSE")
        entry.size = len(license_text)
        archive.addfile(entry, io.BytesIO(license_text))
    manifest = {"project": upload.PROJECT_NAME, "package": upload.PACKAGE_NAME,
                "repository": upload.REPOSITORY, "version": "1.0.1", "tag": "v1.0.1",
                "artifacts": [{"name": artifact.name, "bytes": artifact.stat().st_size,
                               "sha256": upload.file_hash(artifact)}]}
    (directory / "release-manifest.json").write_text(json.dumps(manifest), encoding="utf-8")
    inventory = [artifact, directory / "release-manifest.json"]
    examples = directory / "motion-input-grid-1.0.1-javascript-examples.tar.gz"
    payload = {"LICENSE": b"Fixture license", "licenses/MediaPipe-LICENSE": b"MediaPipe license",
               "licenses/nlohmann-LICENSE": b"JSON license"}
    payload["manifest.json"] = json.dumps({
        "project": upload.PROJECT_NAME, "package": upload.PACKAGE_NAME,
        "repository": upload.REPOSITORY,
        "sha256": {name: upload.streamed_hash(io.BytesIO(data)) for name, data in payload.items()}
    }).encode()
    with tarfile.open(examples, "w:gz") as archive:
        for name, data in payload.items():
            entry = tarfile.TarInfo("examples/" + name)
            entry.size = len(data)
            archive.addfile(entry, io.BytesIO(data))
    manifest["artifacts"].append({"name": examples.name, "bytes": examples.stat().st_size,
                                  "sha256": upload.file_hash(examples)})
    inventory[1].write_text(json.dumps(manifest), encoding="utf-8")
    inventory.append(examples)
    (directory / "SHA256SUMS").write_text("".join(
        f"{upload.file_hash(path)}  {path.name}\n" for path in inventory), encoding="utf-8")
    return inventory + [directory / "SHA256SUMS"]


class ReleaseUploadTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="mig-upload-tests-")
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def test_manifest_hashes_version_and_unlisted_payloads(self):
        files = fixture(self.root)
        self.assertEqual(set(upload.verify_inventory(self.root, "1.0.1")), set(files))
        with self.assertRaisesRegex(ValueError, "version"):
            upload.verify_inventory(self.root, "1.0.2")
        extra = self.root / "unexpected.txt"
        extra.write_text("unlisted")
        with self.assertRaisesRegex(ValueError, "Unlisted"):
            upload.verify_inventory(self.root, "1.0.1")
        extra.unlink()
        files[0].write_bytes(b"corrupt")
        with self.assertRaisesRegex(ValueError, "size/hash mismatch"):
            upload.verify_inventory(self.root, "1.0.1")

    def test_complete_checksum_coverage(self):
        files = fixture(self.root)
        checksums = self.root / "SHA256SUMS"
        original = checksums.read_text()
        checksums.write_text(original.splitlines()[0] + "\n")
        with self.assertRaisesRegex(ValueError, "complete release inventory"):
            upload.verify_inventory(self.root, "1.0.1")
        checksums.write_text(original + f"{'0' * 64}  ../outside\n")
        with self.assertRaisesRegex(ValueError, "Unexpected"):
            upload.verify_inventory(self.root, "1.0.1")
        manifest = json.loads(files[1].read_text())
        manifest["artifacts"].append(manifest["artifacts"][0])
        files[1].write_text(json.dumps(manifest))
        with self.assertRaisesRegex(ValueError, "duplicate"):
            upload.verify_inventory(self.root, "1.0.1")

    def test_missing_javascript_examples_is_an_incomplete_release(self):
        files = fixture(self.root)
        manifest = json.loads(files[1].read_text())
        manifest["artifacts"] = manifest["artifacts"][:1]
        files[1].write_text(json.dumps(manifest))
        with self.assertRaisesRegex(ValueError, "Incomplete release: missing.*javascript-examples"):
            upload.verify_inventory(self.root, "1.0.1")

    def test_duplicate_downloads_and_unsafe_paths(self):
        first = self.root / "motion-input-grid-first-candidate.zip"
        second = self.root / "motion-input-grid-release-candidates.zip"
        for path in (first, second):
            with zipfile.ZipFile(path, "w") as archive:
                archive.writestr("payload.zip", b"identical")
        output = self.root / "extracted"
        output.mkdir()
        upload.extract_downloads(upload.downloaded_archives(self.root), output)
        self.assertEqual((output / "payload.zip").read_bytes(), b"identical")
        with zipfile.ZipFile(second, "w") as archive:
            archive.writestr("payload.zip", b"different")
        with self.assertRaisesRegex(ValueError, "Conflicting"):
            upload.extract_downloads([second], output)
        for name in ("../outside", "C:/outside", "bad:stream", "sub/file"):
            with zipfile.ZipFile(second, "w") as archive:
                archive.writestr(name, b"unsafe")
            with self.assertRaisesRegex(ValueError, "archive path"):
                upload.extract_downloads([second], output)
        (self.root / "motion-input-grid-other-candidate.zip.opdownload").touch()
        with self.assertRaisesRegex(ValueError, "Incomplete"):
            upload.downloaded_archives(self.root)

    def test_ci_must_succeed_for_the_selected_tag_commit(self):
        run = {"path": ".github/workflows/release-check.yml", "head_sha": "commit",
               "head_branch": "v1.0.1", "status": "completed", "conclusion": "success",
               "html_url": "https://github.com/Robin-G0/MIG/actions/runs/1"}
        responses = [{"object": {"type": "tag", "sha": "annotation"}},
                     {"object": {"type": "commit", "sha": "commit"}},
                     [{"workflow_runs": [run]}]]
        with patch.object(upload, "api", side_effect=responses):
            upload.verify_remote_run("v1.0.1")
        for key, value in (("head_sha", "other"), ("head_branch", "main"),
                           ("conclusion", "failure")):
            changed = dict(run, **{key: value})
            with patch.object(upload, "api", side_effect=[
                    {"object": {"type": "commit", "sha": "commit"}},
                    [{"workflow_runs": [changed]}]]):
                with self.assertRaises(ValueError):
                    upload.verify_remote_run("v1.0.1")

    def test_draft_creation_resume_and_conflicts(self):
        files = fixture(self.root)
        release = {"draft": True, "html_url": "https://github.com/Robin-G0/MIG/releases/tag/v1.0.1",
                   "assets": [{"name": path.name, "size": path.stat().st_size,
                               "digest": "sha256:" + upload.file_hash(path)} for path in files]}
        with patch.object(upload, "verify_remote_run"), patch.object(upload, "gh") as commands:
            with patch.object(upload, "remote_release", side_effect=[None, release]):
                upload.upload_release("v1.0.1", files)
            self.assertEqual(commands.call_count, 6)
            self.assertIn("--draft", commands.call_args_list[0].args)
            self.assertIn("--verify-tag", commands.call_args_list[0].args)
            self.assertEqual(commands.call_args_list[-1].args[:2], ("release", "edit"))
            self.assertIn("--notes-file", commands.call_args_list[-1].args)
            commands.reset_mock()
            release["body"] = upload.merge_download_notes(None, "v1.0.1",
                [asset["name"] for asset in release["assets"]])
            with patch.object(upload, "remote_release", return_value=release):
                upload.upload_release("v1.0.1", files)
            commands.assert_not_called()
            release["assets"][0]["digest"] = "sha256:" + "0" * 64
            with patch.object(upload, "remote_release", return_value=release):
                with self.assertRaisesRegex(ValueError, "refusing to replace"):
                    upload.upload_release("v1.0.1", files)
            commands.assert_not_called()

    def test_download_categories_cover_assets_and_preserve_release_notes(self):
        names = [
            'motion-input-grid-1.0.2-windows-x64-native.zip',
            'motion-input-grid-1.0.2-linux-x64-examples.tar.gz',
            'motion-input-grid-1.0.2-windows-x64-python-tkinter-standalone.zip',
            'motion-input-grid-1.0.2-linux-x64-sdl2-standalone.tar.gz',
            'motion-input-grid-1.0.2-browser-react-standalone.tar.gz',
            'motion-input-grid-1.0.2-javascript-examples.tar.gz',
            'motion-input-grid-1.0.2-windows-x64-godot-csharp-standalone.zip',
            'motion-input-grid-1.0.2-linux-arm64-sdk.tar.gz',
            'motion_input_grid-1.0.2-py3-none-win_amd64.whl',
            'motion-input-grid-1.0.2.tgz', 'motion-input-grid_1.0.2_amd64.deb',
            'motion-input-grid-1.0.2-source.tar.gz', 'SHA256SUMS', 'release-manifest.json',
        ]
        original = '## Highlights\n\nA user-written explanation.\n'
        body = upload.merge_download_notes(original, 'v1.0.2', names)
        self.assertIn(original, body)
        for heading in ('Desktop applications', 'Native examples', 'Python Tkinter examples',
                        'SDL2 examples', 'Browser examples', 'Game engines',
                        'SDKs and language packages', 'Source and integrity files'):
            self.assertIn(f'# {heading}\n', body)
        for guide in ('readme.md', 'docs/fr/readme.fr.md'):
            for fragment in re.findall(r'releases/latest#([a-z0-9-]+)',
                                       (ROOT / guide).read_text(encoding='utf-8')):
                self.assertTrue(fragment.startswith('user-content-'), fragment)
                anchor = fragment.removeprefix('user-content-')
                self.assertIn(f'<a name="{anchor}"></a>', body)
        for name in names:
            self.assertIn(f'/releases/download/v1.0.2/{name})', body)
        self.assertEqual(body, upload.merge_download_notes(body, 'v1.0.2', names))
        changed = upload.merge_download_notes(body, 'v1.0.2', names[1:])
        self.assertNotIn(names[0], changed)
        self.assertIn(original, changed)
        # Older releases link to the published collection when individual
        # tutorials do not exist; never invent an unavailable download.
        legacy = upload.merge_download_notes(None, 'v1.0.2', [names[1]])
        self.assertIn('examples/python-tkinter/', legacy)
        self.assertNotIn('python-tkinter-standalone.zip', legacy)
    def test_existing_asset_fallback_and_immutable_release(self):
        files = fixture(self.root)
        asset = {"name": files[0].name, "size": files[0].stat().st_size}
        release = {"assets": [asset], "immutable": True}

        def download(*arguments):
            target = Path(arguments[arguments.index("--dir") + 1]) / files[0].name
            target.write_bytes(files[0].read_bytes())

        with patch.object(upload, "gh", side_effect=download):
            self.assertEqual(upload.missing_assets("v1.0.1", files[:1], release), [])
            with self.assertRaisesRegex(ValueError, "immutable"):
                upload.missing_assets("v1.0.1", files, release)


if __name__ == "__main__":
    unittest.main()
