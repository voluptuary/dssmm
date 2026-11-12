#!/usr/bin/env python3
"""Dispatch an explicit hardware project in the common top-level layout."""
import argparse
import json
from pathlib import Path
import subprocess
from project_policy import validate_layout

ROOT = Path(__file__).resolve().parents[1]
TARGETS = ('ezfomegade', 'ezfomega', 'ezfblue')


def project_root(target):
    records = json.loads((ROOT / 'projects.json').read_text())['projects']
    if target not in records:
        raise ValueError('This branch does not contain ' + target)
    record = records[target]
    path = (ROOT / record['directory']).resolve()
    if record['directory'] != target or path.parent != ROOT or (ROOT / target).is_symlink():
        raise ValueError('Project must be its own direct child directory')
    if not (path / 'project.json').is_file():
        raise ValueError('Project is outside this sparse view; run git sparse-checkout add ' + target)
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=('build', 'verify'))
    parser.add_argument('target', choices=TARGETS)
    args = parser.parse_args()
    validate_layout(ROOT, args.target)
    path = project_root(args.target)
    if args.command == 'build':
        command = ['make']
        if args.target == 'ezfomega':
            command += ['BUILD=build-omega', 'TARGET=omega-port']
    else:
        verifier = {'ezfomegade': 'verify-ezfomegade.sh',
                    'ezfomega': 'verify-omega-port.sh',
                    'ezfblue': 'verify-blue-port.sh'}[args.target]
        command = ['bash', 'scripts/' + verifier]
    print(args.target + ': ' + str(path) + ' -> ' + ' '.join(command), flush=True)
    subprocess.run(command, cwd=path, check=True)


if __name__ == '__main__':
    try:
        main()
    except ValueError as error:
        raise SystemExit(str(error))
