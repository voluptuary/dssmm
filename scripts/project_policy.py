#!/usr/bin/env python3
"""Validate the shared tracked tree, sparse working views and hardware PR scope."""
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


def validate_pr(base, head, changed_paths=None):
    if head in TARGETS and base in TARGETS and head != base:
        raise ValueError('Integrate hardware branches through dssmm, not directly across models')
    if head in TARGETS and base == 'dssmm':
        if changed_paths is None:
            raise ValueError('Hardware PR scope requires base and head commit IDs')
        foreign = [name for name in changed_paths if not name.startswith(head + '/')]
        if foreign:
            raise ValueError('Hardware PR changes files outside ' + head + '/: ' + ', '.join(foreign[:8]))
    if base in TARGETS and head and head != 'dssmm':
        if changed_paths is None:
            raise ValueError('Hardware feature PR scope requires base and head commit IDs')
        if any(not name.startswith(base + '/') for name in changed_paths):
            raise ValueError('Feature PR must stay inside ' + base + '/; route shared changes through dssmm')
    # dssmm -> hardware is an integration sync of the same complete tracked tree.


def changed_since_base(root, base_ref, head_ref):
    ancestor = subprocess.check_output(['git', '-C', str(root), 'merge-base', base_ref, head_ref], text=True).strip()
    # --no-renames includes both sides of a moved file, preventing scope escapes.
    raw = subprocess.check_output(['git', '-C', str(root), 'diff', '--name-only', '--no-renames', '-z', ancestor, head_ref, '--'])
    return [name.decode() for name in raw.split(b'\0') if name]


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
    if set(records) != TARGETS:
        raise ValueError('Every branch must track all three projects in the common registry')
    raw = subprocess.check_output(['git', '-C', str(root), 'ls-files', '-t', '-z'])
    entries = {entry[2:].decode(): entry[:1] == b'S' for entry in raw.split(b'\0') if entry}
    misplaced = {Path(name).parts[0] for name in entries} - UNIVERSAL - TARGETS
    if misplaced:
        raise ValueError('Non-universal root content: ' + ', '.join(sorted(misplaced)))
    for name in entries:
        if name.startswith('scripts/') and name not in SHARED_TOOLS:
            raise ValueError('Hardware-specific root tool: ' + name)
    selected = expected or (branch if branch in TARGETS else None)
    for target, record in records.items():
        directory = root / record['directory']
        if record['directory'] != target or directory.is_symlink() or directory.resolve().parent != root.resolve():
            raise ValueError('Project must be a direct root child with its matching name')
        tracked = {name: skipped for name, skipped in entries.items() if name.startswith(target + '/')}
        if not tracked:
            raise ValueError('Missing tracked project: ' + target)
        if (directory / 'project.json').is_file():
            validate_project(directory, target)
        elif target == selected or not all(tracked.values()):
            raise ValueError('Project missing from checkout or selected outside sparse view: ' + target + '; use git sparse-checkout add ' + target)
    for name in ('AGENTS.md', 'HANDOFF.md', 'PROGRESS.md', 'Makefile', 'LICENSE'):
        if not (root / name).is_file():
            raise ValueError('Missing common root requirement: ' + name)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--expected-target', choices=sorted(TARGETS))
    parser.add_argument('--branch', default='')
    parser.add_argument('--base-branch', default='')
    parser.add_argument('--head-branch', default='')
    parser.add_argument('--base-ref', default='')
    parser.add_argument('--head-ref', default='')
    parser.add_argument('--integration-ref', default='')
    args = parser.parse_args()
    branch = args.branch or subprocess.run(['git', 'symbolic-ref', '--quiet', '--short', 'HEAD'], capture_output=True, text=True).stdout.strip()
    changed = changed_since_base(ROOT, args.base_ref, args.head_ref) if args.base_ref and args.head_ref else None
    validate_pr(args.base_branch, args.head_branch, changed)
    if args.integration_ref and branch in TARGETS and not args.base_branch:
        validate_pr('dssmm', branch, changed_since_base(ROOT, args.integration_ref, 'HEAD'))
    validate_layout(ROOT, args.expected_target, branch)
    print('Shared tracked layout, sparse view, hardware identity and PR scope checks passed.')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, KeyError, FileNotFoundError) as error:
        raise SystemExit(str(error))
