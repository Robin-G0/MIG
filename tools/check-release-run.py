"""Check a candidate Actions run before an explicitly enabled publication workflow."""
import argparse
import json
import os
import urllib.request


def verify_run(run, commit):
    if run["path"] != ".github/workflows/release-check.yml":
        raise ValueError("Candidate must come from the release validation workflow")
    if run["head_sha"] != commit or run["status"] != "completed" or run["conclusion"] != "success":
        raise ValueError("Candidate must be successful and match the selected tag commit")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run_id", type=int)
    args = parser.parse_args()
    if args.run_id <= 0:
        parser.error("Use a positive Actions run ID")
    repository = os.environ["GITHUB_REPOSITORY"]
    if repository != "Robin-G0/MIG":
        raise ValueError("This publication template is configured for Robin-G0/MIG")
    url = f"https://api.github.com/repos/{repository}/actions/runs/{args.run_id}"
    request = urllib.request.Request(url, headers={
        "Authorization": "Bearer " + os.environ["GH_TOKEN"],
        "Accept": "application/vnd.github+json",
    })
    with urllib.request.urlopen(request, timeout=30) as response:
        verify_run(json.load(response), os.environ["GITHUB_SHA"])
    print("Candidate run matches the selected tag commit and succeeded")


if __name__ == "__main__":
    main()
