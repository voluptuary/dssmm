#!/usr/bin/env python3
"""Reject misplaced firmware, mixed branches, duplicate tools and project escapes."""
from pathlib import Path
import json
import runpy
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
policy = runpy.run_path(str(ROOT / 'scripts/project_policy.py'))


def fixture(root, targets):
    records = {t: {'directory': t} for t in targets}
    (root / 'projects.json').write_text(json.dumps({'projects': records}))
    for name in ('AGENTS.md', 'HANDOFF.md', 'PROGRESS.md', 'Makefile', 'LICENSE'):
        (root / name).write_text('')
    for target in targets:
        project = root / target
        project.mkdir()
        (project / 'project.json').write_text(json.dumps({'id': target}))
        source = project / ('targets/blue/source' if target == 'ezfblue' else 'source')
        source.mkdir(parents=True)
        (source / ('ezbluekernel.c' if target == 'ezfblue' else 'ezkernelnew.c')).write_text('')
        for name in ('AGENTS.md', 'HANDOFF.md', 'PROGRESS.md', 'Makefile'):
            (project / name).write_text('')


class Isolation(unittest.TestCase):
    def validate(self, root, expected=None, branch=None, tracked='README.md\nprojects.json\n'):
        with patch.object(policy['subprocess'], 'check_output', return_value=tracked):
            policy['validate_layout'](root, expected, branch)

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

    def test_single_project_branches(self):
        for target in policy['TARGETS']:
            with tempfile.TemporaryDirectory() as folder:
                root = Path(folder)
                fixture(root, [target])
                self.validate(root, target, target)
                with self.assertRaises(ValueError):
                    self.validate(root, branch='dssmm')
                other = next(t for t in policy['TARGETS'] if t != target)
                with self.assertRaises(ValueError):
                    self.validate(root, other)
                (root / other).mkdir()
                with self.assertRaises(ValueError):
                    self.validate(root, target, target)

    def test_canonical(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            fixture(root, policy['TARGETS'])
            self.validate(root, branch='dssmm')
            with self.assertRaises(ValueError):
                self.validate(root, branch='ezfomega')
            for misplaced in ('source/driver.c', 'scripts/flash-de.py', 'targets/ezfomega/Makefile'):
                with self.assertRaises(ValueError):
                    self.validate(root, tracked=misplaced+'\n')
            link = root / 'ezfomega/source/external'
            link.symlink_to(root / 'ezfomegade/source', target_is_directory=True)
            with self.assertRaises(ValueError):
                self.validate(root)

    def test_identity_and_duplicate_infrastructure(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            fixture(root, ['ezfomega'])
            project = root / 'ezfomega'
            (project / '.github').mkdir()
            with self.assertRaises(ValueError):
                self.validate(root)
            (project / '.github').rmdir()
            (project / 'project.json').write_text('{"id":"ezfomegade"}')
            with self.assertRaises(ValueError):
                self.validate(root)

    def test_old_wrapper_and_symlink_directory(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            fixture(root, ['ezfomega'])
            (root / 'projects.json').write_text('{"projects":{"ezfomega":{"directory":"targets/ezfomega"}}}')
            with self.assertRaises(ValueError):
                self.validate(root)
            (root / 'projects.json').write_text('{"projects":{"ezfomega":{"directory":"ezfomega"}}}')
            (root / 'ezfomega').rename(root / 'real')
            (root / 'ezfomega').symlink_to(root / 'real', target_is_directory=True)
            with self.assertRaises(ValueError):
                self.validate(root)


if __name__ == '__main__':
    unittest.main()
