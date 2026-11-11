#!/usr/bin/env python3
"""Dispatch an explicit hardware project in canonical or standalone layouts."""
import argparse
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
TARGETS = ('ezfomegade', 'ezfomega', 'ezfblue')


def project_root(target):
    if (ROOT / 'projects.json').exists():
        record = json.loads((ROOT / 'projects.json').read_text())['projects'][target]
        path = (ROOT / record['directory']).resolve()
        if not path.is_relative_to(ROOT / 'targets'):
            raise ValueError('Project escapes its target directory')
    else:
        record = json.loads((ROOT / 'project.json').read_text())
        if record['id'] != target:
            raise ValueError('Standalone branch belongs to ' + record['id'])
        path = ROOT
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=('build', 'verify'))
    parser.add_argument('target', choices=TARGETS)
    args = parser.parse_args()
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
    main()
