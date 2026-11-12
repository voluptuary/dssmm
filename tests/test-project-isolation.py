#!/usr/bin/env python3
"""Exercise the common registry, hardware boundaries and PR changed-path checks."""
from pathlib import Path
import json
import runpy
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
policy = runpy.run_path(str(ROOT / 'scripts/project_policy.py'))


def fixture(root):
    records = {t: {'directory': t} for t in sorted(policy['TARGETS'])}
    (root / 'projects.json').write_text(json.dumps({'projects': records}))
    for name in ('AGENTS.md', 'HANDOFF.md', 'PROGRESS.md', 'Makefile', 'LICENSE', 'README.md'):
        (root / name).write_text('')
    for directory in ('docs', 'scripts', 'tests', '.devcontainer', '.github'):
        (root / directory).mkdir()
        (root / directory / '.gitkeep').write_text('') if directory != 'scripts' else None
    for target in records:
        project = root / target
        project.mkdir()
        (project / 'project.json').write_text(json.dumps({'id': target}))
        source = project / ('targets/blue/source' if target == 'ezfblue' else 'source')
        source.mkdir(parents=True)
        (source / ('ezbluekernel.c' if target == 'ezfblue' else 'ezkernelnew.c')).write_text('')
        for name in ('AGENTS.md', 'HANDOFF.md', 'PROGRESS.md', 'Makefile'):
            (project / name).write_text('')


class Isolation(unittest.TestCase):
    def validate(self, root, expected=None, branch=None, extra=None):
        names = [str(p.relative_to(root)) for p in root.rglob('*') if p.is_file() or p.is_symlink()]
        names.extend(extra or [])
        tracked = b''.join(b'H ' + name.encode() + b'\0' for name in names)
        with patch.object(policy['subprocess'], 'check_output', return_value=tracked):
            policy['validate_layout'](root, expected, branch)

    def test_pr_boundaries_and_scope(self):
        for target in policy['TARGETS']:
            policy['validate_pr']('dssmm', target, [target + '/source/fix.c', target + '/PROGRESS.md'])
            policy['validate_pr'](target, 'dssmm')
            policy['validate_pr'](target, 'feature/proposed-fix', [target + '/source/fix.c'])
            with self.assertRaises(ValueError):
                policy['validate_pr'](target, 'feature/proposed-fix', ['README.md'])
            with self.assertRaises(ValueError):
                policy['validate_pr']('dssmm', target)
            for path in ['README.md', 'scripts/project.py', next(t for t in policy['TARGETS'] if t != target) + '/source/file.c']:
                with self.assertRaises(ValueError):
                    policy['validate_pr']('dssmm', target, [path])
            for other in policy['TARGETS'] - {target}:
                with self.assertRaises(ValueError):
                    policy['validate_pr'](other, target, [])

    def test_identical_layout_on_all_branches(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            fixture(root)
            for branch in policy['TARGETS'] | {'dssmm'}:
                self.validate(root, branch=branch)
            for path in ['source/driver.c', 'scripts/flash-de.py', 'targets/ezfomega/Makefile']:
                with self.assertRaises(ValueError):
                    self.validate(root, extra=[path])
            records = {'ezfomega': {'directory': 'ezfomega'}}
            (root / 'projects.json').write_text(json.dumps({'projects': records}))
            with self.assertRaises(ValueError):
                self.validate(root, branch='ezfomega')

    def test_identity_duplicates_and_symlink_escape(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            fixture(root)
            project = root / 'ezfomega'
            (project / '.github').mkdir()
            with self.assertRaises(ValueError):
                self.validate(root)
            (project / '.github').rmdir()
            (project / 'project.json').write_text('{"id":"ezfomegade"}')
            with self.assertRaises(ValueError):
                self.validate(root)
            (project / 'project.json').write_text('{"id":"ezfomega"}')
            (project / 'source/external').symlink_to(root / 'ezfomegade/source', target_is_directory=True)
            with self.assertRaises(ValueError):
                self.validate(root)

    def test_deleted_directory_is_not_sparse_checkout(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            fixture(root)
            names = [str(p.relative_to(root)) for p in root.rglob('*') if p.is_file()]
            import shutil
            shutil.rmtree(root / 'ezfblue')
            tracked = b''.join(b'H ' + name.encode() + b'\0' for name in names)
            with patch.object(policy['subprocess'], 'check_output', return_value=tracked):
                with self.assertRaises(ValueError):
                    policy['validate_layout'](root)


if __name__ == '__main__':
    unittest.main()
