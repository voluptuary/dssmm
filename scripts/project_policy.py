#!/usr/bin/env python3
"""Detect mixed hardware layouts and direct cross-platform branch PRs."""
import argparse
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
TARGETS = {'ezfomegade', 'ezfomega', 'ezfblue'}
UNIVERSAL = {'.devcontainer', '.editorconfig', '.git-blame-ignore-revs', '.gitattributes',
             '.github', '.gitignore', 'AGENTS.md', 'CREDITS.md', 'HANDOFF.md', 'LICENSE',
             'Makefile', 'PROGRESS.md', 'README.md', 'docs', 'projects.json', 'scripts',
             'targets', 'tests'}


def validate_pr(base, head):
    if head in TARGETS and base in TARGETS | {'dssmm'} and head != base:
        raise ValueError('Direct cross-target PR refused; adapt changes inside the destination project')
    if head == 'dssmm' and base in TARGETS:
        raise ValueError('Canonical layout cannot be merged wholesale into a standalone target')


def validate_layout(root, expected=None):
    if (root / 'projects.json').exists():
        records = json.loads((root / 'projects.json').read_text())['projects']
        if set(records) != TARGETS:
            raise ValueError('Canonical layout must contain exactly the three hardware projects')
        tracked = subprocess.check_output(['git', '-C', str(root), 'ls-files'], text=True).splitlines()
        misplaced = {Path(name).parts[0] for name in tracked} - UNIVERSAL
        if misplaced:
            raise ValueError('Non-universal root content: ' + ', '.join(sorted(misplaced)))
        children = {p.name for p in (root / 'targets').iterdir() if p.is_dir()}
        if children != TARGETS:
            raise ValueError('Unexpected hardware project directory')
        for name in tracked:
            if name.startswith('scripts/') and name not in {'scripts/project.py', 'scripts/project_policy.py', 'scripts/build-site.py'}:
                raise ValueError('Hardware-specific root tool: ' + name)
        for target, record in records.items():
            directory = root / record['directory']
            if record['directory'] != 'targets/' + target:
                raise ValueError('Project has an unexpected directory: ' + target)
            if directory.is_symlink() or not directory.resolve().is_relative_to((root / 'targets').resolve()):
                raise ValueError('Project directory escapes its isolation boundary')
            validate_layout(directory, target)
        if expected and expected not in records:
            raise ValueError('Unknown hardware target')
        return
    identity = json.loads((root / 'project.json').read_text())['id']
    if identity not in TARGETS or (expected and identity != expected):
        raise ValueError('Hardware identity mismatch')
    foreign = {p.name for p in (root / 'targets').iterdir() if p.is_dir()} if (root / 'targets').exists() else set()
    if foreign != ({'blue'} if identity == 'ezfblue' else set()):
        raise ValueError('Standalone branch contains foreign project directories')
    if identity == 'ezfblue':
        if (root / 'source').exists() or not (root / 'targets/blue/source/ezbluekernel.c').exists():
            raise ValueError('BLUE must contain only its own firmware source')
    elif not (root / 'source/ezkernelnew.c').exists() or (root / 'targets/blue/source').exists():
        raise ValueError('DE/OMEGA layout has missing or foreign firmware')
    for path in root.rglob('*'):
        if path.is_symlink() and not path.resolve().is_relative_to(root.resolve()):
            raise ValueError('Project contains an external symlink: ' + str(path))
    for name in ('AGENTS.md', 'HANDOFF.md', 'PROGRESS.md', 'Makefile'):
        if not (root / name).is_file():
            raise ValueError('Missing project requirement: ' + name)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--expected-target', choices=sorted(TARGETS))
    parser.add_argument('--base-branch', default='')
    parser.add_argument('--head-branch', default='')
    args = parser.parse_args()
    validate_pr(args.base_branch, args.head_branch)
    validate_layout(ROOT, args.expected_target)
    print('Project identity/layout and PR direction checks passed; no automatic synchronization.')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, KeyError, FileNotFoundError) as error:
        raise SystemExit(str(error))
