#!/usr/bin/env python3
"""Exercise branch-direction and misplaced/cross-linked hardware rejection."""
from pathlib import Path
import json
import runpy
import tempfile
import unittest
from unittest.mock import patch
ROOT = Path(__file__).resolve().parents[1]
policy = runpy.run_path(str(ROOT / 'scripts/project_policy.py'))


def make_project(root, target):
    root.mkdir(parents=True, exist_ok=True)
    (root / 'project.json').write_text(json.dumps({'id': target}))
    source = root / ('targets/blue/source' if target == 'ezfblue' else 'source')
    source.mkdir(parents=True)
    (source / ('ezbluekernel.c' if target == 'ezfblue' else 'ezkernelnew.c')).write_text('')
    for name in ['AGENTS.md', 'HANDOFF.md', 'PROGRESS.md', 'Makefile']:
        (root / name).write_text('')


class Isolation(unittest.TestCase):
    def test_pr_boundaries(self):
        for head in policy['TARGETS']:
            for base in policy['TARGETS'] | {'dssmm'}:
                if base == head:
                    policy['validate_pr'](base, head)
                else:
                    with self.assertRaises(ValueError):
                        policy['validate_pr'](base, head)
            with self.assertRaises(ValueError):
                policy['validate_pr'](head, 'dssmm')
            policy['validate_pr'](head, 'feature/proposed-fix')

    def test_standalone(self):
        for target in policy['TARGETS']:
            with tempfile.TemporaryDirectory() as folder:
                root = Path(folder)
                make_project(root, target)
                policy['validate_layout'](root, target)
                with self.assertRaises(ValueError):
                    policy['validate_layout'](root, next(t for t in policy['TARGETS'] if t != target))
                (root / 'targets/foreign').mkdir(parents=True)
                with self.assertRaises(ValueError):
                    policy['validate_layout'](root, target)

    def test_canonical(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            records = {t: {'directory': 'targets/' + t} for t in policy['TARGETS']}
            (root / 'projects.json').write_text(json.dumps({'projects': records}))
            for target in records:
                make_project(root / records[target]['directory'], target)
            with patch.object(policy['subprocess'], 'check_output', return_value='README.md\nprojects.json\n'):
                policy['validate_layout'](root)
            for misplaced in ['source/driver.c', 'scripts/flash-de.py']:
                with patch.object(policy['subprocess'], 'check_output', return_value=misplaced+'\n'):
                    with self.assertRaises(ValueError):
                        policy['validate_layout'](root)
            source = root / 'targets/ezfomega/source/external'
            source.symlink_to(root / 'targets/ezfomegade/source', target_is_directory=True)
            with patch.object(policy['subprocess'], 'check_output', return_value='README.md\n'):
                with self.assertRaises(ValueError):
                    policy['validate_layout'](root)


if __name__ == '__main__':
    unittest.main()
