#!/usr/bin/env python3
"""Report the approximate context cost of agent-facing documentation.

Advisory by default. `--check` exits nonzero when a set exceeds its budget, so a
release checkpoint or CI job can catch unbounded journal growth.
"""
import argparse
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LANES = ('ezfomegade', 'ezfomega', 'ezfblue', 'superfw')
TRIO = ('AGENTS.md', 'HANDOFF.md', 'PROGRESS.md')


def measure(paths):
    total = 0
    lines = 0
    for path in paths:
        if path.is_file():
            data = path.read_bytes()
            total += len(data)
            lines += data.count(b'\n')
    return total, lines


def dir_bytes(path):
    if not path.is_dir():
        return 0
    return sum(item.stat().st_size for item in path.rglob('*') if item.is_file())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--max-root-tokens', type=int, default=20000)
    parser.add_argument('--max-lane-tokens', type=int, default=60000)
    parser.add_argument('--check', action='store_true',
                        help='exit nonzero when a budget is exceeded')
    args = parser.parse_args()

    rows = [('root', [ROOT / name for name in TRIO])]
    rows += [(lane, [ROOT / lane / name for name in TRIO]) for lane in LANES if (ROOT / lane).is_dir()]
    warnings = []
    print('%-12s %10s %10s %8s  %s' % ('set', 'bytes', '~tokens', 'lines', 'budget'))
    for name, paths in rows:
        size, lines = measure(paths)
        tokens = size // 4
        limit = args.max_root_tokens if name == 'root' else args.max_lane_tokens
        flag = 'OVER' if tokens > limit else 'ok'
        print('%-12s %10d %10d %8d  %s (%d)' % (name, size, tokens, lines, flag, limit))
        if tokens > limit:
            warnings.append('%s documentation is ~%d tokens (budget %d)' % (name, tokens, limit))

    history = dir_bytes(ROOT / 'docs' / 'history')
    history += sum(dir_bytes(ROOT / lane / 'docs' / 'history') for lane in LANES)
    print('\nArchived history (search-excluded): %.1f MB' % (history / 1_000_000))

    if warnings:
        print('\nWarnings:')
        for warning in warnings:
            print(' - ' + warning)
        print('Compact the current-state summary, not the historical record.')
    else:
        print('\nAll documentation sets are within budget.')

    return 1 if (warnings and args.check) else 0


if __name__ == '__main__':
    raise SystemExit(main())
