# DSSMM shared-tree working architecture

Repository: https://github.com/voluptuary/dssmm. Default/integration branch:
`dssmm`. Hardware branches: `ezfomegade`, `ezfomega`, `ezfblue`. Do not recreate
retired `main` or `ezfode` branches.

## One tracked layout, separate hardware ownership

Every branch tracks the SAME complete tree: universal root infrastructure plus
`ezfomegade/`, `ezfomega/`, and `ezfblue/`. Hardware branches are development lanes,
not different repository layouts. GitHub therefore shows all three projects on
every branch. Optional worktree-local sparse checkout hides unrelated projects
without deleting them from Git. Never commit their removal to focus a branch.

On a hardware branch, edit only its matching directory, including that project's
AGENTS.md/HANDOFF.md/PROGRESS.md. Read those files before target work. Hardware
sources, protocols, assets, settings and tests remain private to their directory;
never symlink/copy another model's driver into a project without explicit review.
Root contains strictly universal environment/build/CI/policy/docs/licensing.

Root AGENTS/HANDOFF/PROGRESS cover shared architecture and integration milestones.
Do NOT update root logs for routine single-project work: that creates unnecessary
cross-branch conflicts. Keep platform findings and progress in the owning project.
Preserve every historical handoff/progress log. Pre-conversion branch records
are archived under each project's docs/history/pre-shared-tree/. Earlier layout
and merge prohibitions in historical records are superseded by this architecture.

## Normal merge workflow

1. Develop/test on the matching hardware branch, confined to its directory.
2. Merge that branch normally into dssmm (or use a PR). CI checks hardware PR
   changed paths against the merge base and verifies all projects on dssmm.
3. Merge dssmm back into hardware branches to synchronize the combined tree.
   A fast-forward is preferred when possible; never reset/force-push for syncing.
4. Make universal root changes on dssmm or a shared feature branch, then sync
   them outward. Do not mix root changes into a hardware-to-dssmm PR.

Ordinary directory-confined changes no longer add/remove other platforms during
merges. Git can still conflict on concurrent edits to the same file; resolve
those explicitly. Direct hardware-to-different-hardware PRs remain rejected:
route integration through dssmm. Do not install blanket ours/theirs merge drivers
or bypass failing checks. The one-time historical ancestry bridge is documented;
future merges must use ordinary Git merge behavior.

## Working views and builds

A hardware worktree can be focused with:
`git sparse-checkout set --cone .devcontainer .github docs scripts tests ezfomega`
(substitute its target). Root files remain visible automatically. Sparse settings
are worktree-local; `git sparse-checkout disable` restores the full view. Never
hand-delete tracked directories. Shared CI uses complete checkouts.

`make` prints help. `make ezfomegade`, `make ezfomega`, `make ezfblue` explicitly
select hardware. `make build-all`/`make verify-all` select projects materialized
in the current working view; on full dssmm they select all three. For a hidden
project, expand the sparse view first. Outputs remain private to the project.
Run `make check-isolation`, `python3 tests/test-project-isolation.py` and
`python3 tests/test-project-merges.py` when changing this architecture. Root CI
is read-only; Pages publication remains manual/default-only.

projects.json is a common live registry, not independent frozen snapshots.
Its baseline commits record known kernel inputs. `scripts/check-snapshots.py`
checks those source/assets/build/test baselines (including sparse index inputs),
not live documentation. For intentional later firmware changes, update baselines
only after appropriate validation; retain historical verification reports.

DE8.3.2 remains stable/latest for Definitive Edition Rev.A/B. Original OMEGA
Test3 and BLUE Test2 remain hardware-untested prereleases. Builds, mocks and
static layout bounds do not qualify physical flash/save timing or runtime stack
peaks. No release changes are authorized merely by architecture maintenance.
Preserve credits, tags/assets, user edits and all journals. Use automatic GitHub
source archives, never upload extra source ZIPs. Candidate folders were retired.

Checkpoint before substantial shared changes and after milestones, recording
actual tests/results and exact continuation steps. Commit/push within the right
branch; verify remote state. Branch protection enforcement is a repository
setting, not a guarantee supplied by directory layout or CI.
