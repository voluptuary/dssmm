#!/usr/bin/env python3
"""Check kernel inputs against recorded baselines without freezing live project docs."""
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
DOC_NAMES = {'AGENTS.md', 'HANDOFF.md', 'PROGRESS.md', 'README.md', 'CREDITS.md', 'CHANGELOG.md', 'project.json', 'COPYING'}


def kernel_input(name):
    return not (name in DOC_NAMES or name.startswith('docs/'))


def git(*args):
    return subprocess.check_output(['git', '-C', str(ROOT), *args])


results = []
for target, record in json.loads((ROOT / 'projects.json').read_text())['projects'].items():
    prefix = record['directory'] + '/'
    source_prefix = record['baseline_directory'] + '/'
    expected_names = set()
    checked = 0
    raw = git('ls-tree', '-rz', record['baseline_commit'], '--', source_prefix)
    if not raw:
        raise SystemExit('Empty kernel-input baseline: ' + target)
    for entry in raw.split(b'\0'):
        if not entry:
            continue
        metadata, full = entry.split(b'\t', 1)
        mode, kind, expected = metadata.decode().split()
        name = full.decode()[len(source_prefix):]
        if not kernel_input(name):
            continue
        expected_names.add(name)
        if kind != 'blob' or mode not in ('100644', '100755'):
            raise SystemExit('Unexpected baseline entry type: ' + name)
        path = ROOT / prefix / name
        if path.exists():
            actual = git('hash-object', '--path=' + prefix + name, str(path)).decode().strip()
            actual_mode = '100755' if path.stat().st_mode & 0o111 else '100644'
        else:
            # Sparse checkout retains all projects in the index without materializing them.
            stage = git('ls-files', '-t', '-s', '--', prefix + name).decode().split()
            if len(stage) < 4 or stage[0] != 'S':
                raise SystemExit('Missing kernel input: ' + prefix + name)
            actual_mode, actual = stage[1:3]
        if actual != expected or actual_mode != mode:
            raise SystemExit('Kernel input differs from recorded baseline: ' + prefix + name)
        checked += 1
    tracked = git('ls-files', '-z', '--', record['directory']).split(b'\0')
    actual_names = {name.decode()[len(prefix):] for name in tracked if name and kernel_input(name.decode()[len(prefix):])}
    if actual_names != expected_names:
        raise SystemExit('Kernel input inventory differs from baseline: ' + target)
    results.append({'target': target, 'baseline_commit': record['baseline_commit'], 'unchanged_kernel_inputs': checked})
print(json.dumps(results, indent=2))
