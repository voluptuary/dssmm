#!/usr/bin/env python3
"""Test ordinary Git merges, scope escapes and worktree-local sparse integration."""
from pathlib import Path
import runpy
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
helpers = runpy.run_path(str(ROOT / 'tests/test-project-isolation.py'))
policy = helpers['policy']
TARGETS = sorted(policy['TARGETS'])


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args], stderr=subprocess.PIPE).decode().strip()


def commit(root, message):
    git(root, 'add', '-A')
    git(root, 'commit', '-qm', message)


class Merges(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name) / 'repo'
        self.root.mkdir()
        helpers['fixture'](self.root)
        git(self.root, 'init', '-q', '-b', 'dssmm')
        git(self.root, 'config', 'user.name', 'Merge Fixture')
        git(self.root, 'config', 'user.email', 'fixture@example.invalid')
        commit(self.root, 'Shared complete baseline')
        for target in TARGETS:
            git(self.root, 'branch', target)

    def test_independent_hardware_merges_and_reverse_sync(self):
        before = {target: git(self.root, 'rev-parse', 'HEAD:' + target) for target in TARGETS}
        for target in TARGETS:
            git(self.root, 'switch', target)
            (self.root / target / 'PROGRESS.md').write_text('Target-specific work\n')
            (self.root / target / 'source-change.fixture').write_text(target + '\n')
            commit(self.root, target + ' isolated change')
            paths = policy['changed_since_base'](self.root, 'dssmm', target)
            policy['validate_pr']('dssmm', target, paths)
            self.assertTrue(all(p.startswith(target + '/') for p in paths))
            git(self.root, 'switch', 'dssmm')
            previous = {other: git(self.root, 'rev-parse', 'HEAD:' + other) for other in TARGETS if other != target}
            git(self.root, 'merge', '--no-ff', '--no-edit', target)
            for other, tree in previous.items():
                self.assertEqual(tree, git(self.root, 'rev-parse', 'HEAD:' + other))
            policy['validate_layout'](self.root, branch='dssmm')
        for target in TARGETS:
            self.assertNotEqual(before[target], git(self.root, 'rev-parse', 'HEAD:' + target))
        (self.root / 'README.md').write_text('Shared architecture update\n')
        commit(self.root, 'Common environment documentation')
        integrated = git(self.root, 'rev-parse', 'HEAD^{tree}')
        for target in TARGETS:
            git(self.root, 'switch', target)
            git(self.root, 'merge', '--no-edit', 'dssmm')
            self.assertEqual(integrated, git(self.root, 'rev-parse', 'HEAD^{tree}'))
            policy['validate_layout'](self.root, branch=target)

    def test_renames_and_deletions_cannot_escape_hardware_scope(self):
        git(self.root, 'switch', 'ezfomega')
        git(self.root, 'mv', 'ezfomegade/source/ezkernelnew.c', 'ezfomega/stolen.c')
        commit(self.root, 'Cross-project rename must fail scope')
        changed = policy['changed_since_base'](self.root, 'dssmm', 'ezfomega')
        self.assertIn('ezfomegade/source/ezkernelnew.c', changed)
        with self.assertRaises(ValueError):
            policy['validate_pr']('dssmm', 'ezfomega', changed)

    def test_sparse_worktree_merge_preserves_hidden_projects(self):
        focus = Path(self.temp.name) / 'focus'
        git(self.root, 'worktree', 'add', '-q', str(focus), 'ezfomega')
        git(focus, 'sparse-checkout', 'set', '--cone', '.devcontainer', '.github', 'docs', 'scripts', 'tests', 'ezfomega')
        self.assertTrue((focus / 'ezfomega/project.json').is_file())
        self.assertFalse((focus / 'ezfblue').exists())
        self.assertTrue((self.root / 'ezfblue/project.json').is_file())
        policy['validate_layout'](focus, expected='ezfomega', branch='ezfomega')
        (focus / 'ezfomega/PROGRESS.md').write_text('Omega work from sparse checkout\n')
        commit(focus, 'Isolated sparse change')
        (self.root / 'ezfblue/PROGRESS.md').write_text('BLUE update, hidden in Omega worktree\n')
        commit(self.root, 'Independent BLUE integration')
        git(focus, 'merge', '--no-ff', '--no-edit', 'dssmm')
        self.assertFalse((focus / 'ezfblue').exists())
        self.assertEqual('BLUE update, hidden in Omega worktree', git(focus, 'show', 'HEAD:ezfblue/PROGRESS.md'))
        self.assertTrue((focus / 'ezfomega/PROGRESS.md').read_text().startswith('Omega work'))
        policy['validate_layout'](focus, expected='ezfomega', branch='ezfomega')
        with self.assertRaises(ValueError):
            policy['validate_layout'](focus, expected='ezfblue')
        git(self.root, 'merge', '--no-ff', '--no-edit', 'ezfomega')
        self.assertEqual(git(self.root, 'rev-parse', 'HEAD^{tree}'), git(focus, 'rev-parse', 'HEAD^{tree}'))
        git(focus, 'sparse-checkout', 'disable')
        self.assertTrue((focus / 'ezfblue/project.json').is_file())
        policy['validate_layout'](focus, branch='ezfomega')


if __name__ == '__main__':
    unittest.main()
