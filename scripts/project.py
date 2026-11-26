#!/usr/bin/env python3
"""Dispatch an explicit hardware project in the common top-level layout."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
from project_policy import validate_layout

ROOT = Path(__file__).resolve().parents[1]
TARGETS = ('ezfomegade', 'ezfomega', 'ezfblue', 'superfw')
LOG_DIR = ROOT / 'build'
PASS_TAIL = 6
FAIL_TAIL = 60


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


def run_logged(command, cwd, label):
    """Run a build/verify command, keep the full log, and print a bounded summary.

    CI and DSSMM_VERBOSE stream everything, as before. Interactive runs get a
    verdict plus the last few lines so an agent does not ingest the whole log.
    """
    LOG_DIR.mkdir(parents=True, exist_ok=True)
    log_path = LOG_DIR / (label + '.log')
    stream = bool(os.environ.get('CI') or os.environ.get('DSSMM_VERBOSE'))
    lines = []
    with log_path.open('w', encoding='utf-8', errors='replace') as log:
        process = subprocess.Popen(
            command, cwd=str(cwd), stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT, text=True, encoding='utf-8',
            errors='replace', bufsize=1)
        assert process.stdout is not None
        for line in process.stdout:
            log.write(line)
            lines.append(line)
            if stream:
                sys.stdout.write(line)
        process.wait()
    status = 'PASS' if process.returncode == 0 else 'FAIL'
    print('%s: %s (log: %s, %d lines)' % (
        label, status, log_path.relative_to(ROOT), len(lines)), flush=True)
    tail = lines[-FAIL_TAIL:] if process.returncode else lines[-PASS_TAIL:]
    if tail:
        sys.stdout.write(''.join(tail))
        if not tail[-1].endswith('\n'):
            sys.stdout.write('\n')
    return process.returncode


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=('build', 'verify'))
    parser.add_argument('target', choices=TARGETS)
    parser.add_argument('--board', choices=('sd', 'lite', 'chis'), default='sd')
    args = parser.parse_args()
    validate_layout(ROOT, args.target)
    path = project_root(args.target)
    if args.command == 'build':
        if args.target == 'superfw':
            command = ['bash', 'scripts/build.sh', args.board]
        else:
            command = ['make']
        if args.target == 'ezfomega':
            command += ['BUILD=build-omega', 'TARGET=omega-port']
    else:
        if args.target == 'superfw':
            command = ['bash', 'scripts/verify.sh']
        else:
            verifier = {'ezfomegade': 'verify-ezfomegade.sh',
                        'ezfomega': 'verify-omega-port.sh',
                        'ezfblue': 'verify-blue-port.sh'}[args.target]
            command = ['bash', 'scripts/' + verifier]
    label = args.command + '-' + args.target
    if args.target == 'superfw' and args.command == 'build':
        label += '-' + args.board
    print(args.target + ': ' + str(path) + ' -> ' + ' '.join(command), flush=True)
    return run_logged(command, path, label)


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except ValueError as error:
        raise SystemExit(str(error))
