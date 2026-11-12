#!/usr/bin/env python3
"""Validate the same top-level project layout on canonical and hardware branches."""
import argparse
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
TARGETS = {'ezfomegade', 'ezfomega', 'ezfblue'}
UNIVERSAL = {'.devcontainer', '.editorconfig', '.git-blame-ignore-revs', '.gitattributes',
             '.github', '.gitignore', 'AGENTS.md', 'CREDITS.md', 'HANDOFF.md', 'LICENSE',
             'Makefile', 'PROGRESS.md', 'README.md', 'docs', 'projects.json', 'scripts', 'tests'}
SHARED_TOOLS = {'scripts/project.py', 'scripts/project_policy.py',
                'scripts/build-site.py', 'scripts/check-snapshots.py'}


def validate_pr(base, head):
    if head in TARGETS and base in TARGETS | {'dssmm'} and head != base:
        raise ValueError('Direct cross-target PR refused; adapt changes inside the destination project')
    if head == 'dssmm' and base in TARGETS:
        raise ValueError('Do not merge a whole multi-project branch into one hardware branch')


def validate_project(root, target):
    identity = json.loads((root / 'project.json').read_text())['id']
    if identity != target:
        raise ValueError('Hardware identity mismatch')
    foreign = {p.name for p in (root / 'targets').iterdir() if p.is_dir()} if (root / 'targets').exists() else set()
    if foreign != ({'blue'} if target == 'ezfblue' else set()):
        raise ValueError('Project contains foreign hardware directories')
    if target == 'ezfblue':
        if (root / 'source').exists() or not (root / 'targets/blue/source/ezbluekernel.c').exists():
            raise ValueError('BLUE must contain only its own firmware source')
    elif not (root / 'source/ezkernelnew.c').exists():
        raise ValueError('Missing DE/OMEGA firmware source')
    for path in root.rglob('*'):
        if path.is_symlink() and not path.resolve().is_relative_to(root.resolve()):
            raise ValueError('Project contains an external symlink: ' + str(path))
    for name in ('AGENTS.md', 'HANDOFF.md', 'PROGRESS.md', 'Makefile'):
        if not (root / name).is_file():
            raise ValueError('Missing project requirement: ' + name)
    for name in ('.github', '.devcontainer', '.gitignore', '.gitattributes', 'scripts/project.py', 'scripts/project_policy.py'):
        if (root / name).exists():
            raise ValueError('Universal infrastructure duplicated inside project: ' + name)


def validate_layout(root, expected=None, branch=None):
    records = json.loads((root / 'projects.json').read_text())['projects']
    if not records or not set(records) <= TARGETS:
        raise ValueError('Unknown or empty project registry')
    required = TARGETS if branch == 'dssmm' else ({branch} if branch in TARGETS else None)
    if required is not None and set(records) != required:
        raise ValueError('Branch contains the wrong hardware project set')
    if expected and expected not in records:
        raise ValueError('Requested hardware is absent from this branch')
    tracked = subprocess.check_output(['git', '-C', str(root), 'ls-files'], text=True).splitlines()
    misplaced = {Path(name).parts[0] for name in tracked} - UNIVERSAL - set(records)
    if misplaced:
        raise ValueError('Non-universal root content: ' + ', '.join(sorted(misplaced)))
    actual = {name for name in TARGETS if (root / name).exists()}
    if actual != set(records):
        raise ValueError('Unexpected or missing top-level hardware directory')
    for name in tracked:
        if name.startswith('scripts/') and name not in SHARED_TOOLS:
            raise ValueError('Hardware-specific root tool: ' + name)
    for target, record in records.items():
        directory = root / record['directory']
        if record['directory'] != target or directory.is_symlink() or directory.resolve().parent != root.resolve():
            raise ValueError('Project must be a direct root child with its matching name')
        validate_project(directory, target)
    for name in ('AGENTS.md', 'HANDOFF.md', 'PROGRESS.md', 'Makefile', 'LICENSE'):
        if not (root / name).is_file():
            raise ValueError('Missing common root requirement: ' + name)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--expected-target', choices=sorted(TARGETS))
    parser.add_argument('--branch', default='')
    parser.add_argument('--base-branch', default='')
    parser.add_argument('--head-branch', default='')
    args = parser.parse_args()
    branch = args.branch or subprocess.run(['git','symbolic-ref','--quiet','--short','HEAD'],capture_output=True,text=True).stdout.strip()
    validate_pr(args.base_branch, args.head_branch)
    validate_layout(ROOT, args.expected_target, branch)
    print('Top-level project identity/layout and PR direction checks passed.')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, KeyError, FileNotFoundError) as error:
        raise SystemExit(str(error))
