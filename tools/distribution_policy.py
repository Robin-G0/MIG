"""Generated directories excluded when assembling release archives."""
EXCLUDED_NAMES = {
    ".git", ".codex", ".agents", ".vscode", ".venv", "__pycache__", "node_modules",
    ".next", "build", "install", "distribution", "dist", "out",
}


def excluded_names():
    return EXCLUDED_NAMES


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--excluded-names', action='store_true', required=True)
    parser.parse_args()
    print('\n'.join(sorted(excluded_names())))
