"""Check first-party guide translations, language links and local Markdown targets."""
from pathlib import Path
import re
from urllib.parse import unquote, urlsplit
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from distribution_policy import private_file, GENERATED_DIRECTORIES

ROOT = Path(__file__).resolve().parents[1]


def guides():
    paths = list(ROOT.glob("*.md"))
    for directory in ("docs", "examples", "bindings", "integrations", "ports"):
        paths.extend((ROOT / directory).rglob("*.md"))
    return sorted(path for path in set(paths) if not private_file(path.name)
                  and not any(part in GENERATED_DIRECTORIES or part in ("licenses", "public")
                              for part in path.relative_to(ROOT).parts)
                  and not path.name.startswith("LICENSE"))


def anchors(path):
    content = re.sub(r"```.*?```", "", path.read_text(encoding="utf-8"), flags=re.S)
    result = set(re.findall(r'(?:id|name)=["\']([^"\']+)["\']', content))
    counts = {}
    for heading in re.findall(r"^#{1,6} +(.+?) *#* *$", content, flags=re.M):
        heading = re.sub(r"<[^>]+>", "", heading).lower()
        slug = "".join(char for char in heading
                       if char.isalnum() or char in " _-").replace(" ", "-")
        count = counts.get(slug, 0)
        counts[slug] = count + 1
        result.add(slug + (f"-{count}" if count else ""))
    return result


def check(path):
    content = path.read_text(encoding="utf-8")
    english_name = path.name.replace(".fr.md", ".md")
    french_name = english_name.removesuffix(".md") + ".fr.md"
    errors = []
    if any(ord(character) < 32 and character not in "\n\r\t" for character in content):
        errors.append("unexpected control character")
    for name in (english_name, french_name):
        if not path.with_name(name).is_file():
            errors.append(f"missing translation: {name}")
        if f"]({name})" not in content:
            errors.append(f"missing language link: {name}")
    outside_code = re.sub(r"```.*?```", "", content, flags=re.S)
    targets = re.findall(r"\]\(([^\s)]+)(?:\s+\"[^\"]*\")?\)", outside_code)
    targets.extend(re.findall(r"^ *\[[^]]+\]: *<?([^\s>]+)>?", outside_code, flags=re.M))
    for target in targets:
        address = urlsplit(target.strip("<>"))
        if address.scheme or target.startswith("//"):
            continue
        destination = path.parent / unquote(address.path) if address.path else path
        if not destination.exists() or private_file(destination.name):
            errors.append(f"missing/publicly excluded link target: {target}")
        elif address.fragment and destination.suffix == ".md":
            if unquote(address.fragment) not in anchors(destination):
                errors.append(f"missing anchor: {target}")
    return errors


def main():
    failures = 0
    paths = guides()
    for path in paths:
        for error in check(path):
            print(f"{path.relative_to(ROOT)}: {error}")
            failures += 1
    if failures:
        raise SystemExit(1)
    print(f"Verified {len(paths)} guides: English/French pairs, switches and local links.")


if __name__ == "__main__":
    main()
