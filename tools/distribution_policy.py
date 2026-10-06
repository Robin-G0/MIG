"""Files excluded from public source and generated distribution payloads."""
PRIVATE_GUIDES = {
    "project-history", "prototype-review", "release-review", "ui-requirements",
    "motion-format", "distribution-validation",
}
PRIVATE_FILES = {"NEXT.md", "archived-NEXT.md", "placeholders.md", "CONTEXT.txt"}
GENERATED_DIRECTORIES = {
    ".git", ".codex", ".agents", ".vscode", ".venv", "__pycache__", "node_modules",
    ".next", "build", "install", "distribution", "dist", "out",
}

GENERATED_PATHS = {
    "bindings/javascript/runtime", "bindings/javascript/LICENSE",
    "examples/react/public/mig", "examples/vue/public/mig", "examples/next/public/mig",
}


def private_file(name):
    stem = name.removesuffix(".md").removesuffix(".fr")
    return name in PRIVATE_FILES or stem in PRIVATE_GUIDES


def excluded_names():
    names = set(PRIVATE_FILES) | GENERATED_DIRECTORIES
    for stem in PRIVATE_GUIDES:
        names.update((stem + '.md', stem + '.fr.md'))
    return names


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--excluded-names', action='store_true', required=True)
    parser.parse_args()
    print('\n'.join(sorted(excluded_names())))
