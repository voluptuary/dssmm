# DSSMM shared-tree working architecture

Repository: https://github.com/voluptuary/dssmm. Default/integration branch:
`dssmm`. Hardware/project branches: `ezfomegade`, `ezfomega`, `ezfblue`, `superfw`. Do not recreate
retired `main` or `ezfode` branches.

## One tracked layout, separate hardware ownership

Every branch tracks the SAME complete tree: universal root infrastructure plus
`ezfomegade/`, `ezfomega/`, `ezfblue/`, and `superfw/`. Project branches are development lanes,
not different repository layouts. GitHub therefore shows all four projects on
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
3. Use merge commits for long-lived hardware PRs (not squash/rebase merging),
   preserving ancestry for repeated integration. Do not delete hardware lanes.
4. Merge dssmm back into hardware branches to synchronize the combined tree.
   A fast-forward is preferred when possible; never reset/force-push for syncing.
5. Make universal root changes on dssmm or a shared feature branch, then sync
   them outward. Do not mix root changes into a hardware-to-dssmm PR.

Ordinary directory-confined changes no longer add/remove other platforms during
merges. Git can still conflict on concurrent edits to the same file; resolve
those explicitly. Direct hardware-to-different-hardware PRs remain rejected:
route integration through dssmm. Do not install blanket ours/theirs merge drivers
or bypass failing checks. The one-time historical ancestry bridge is documented;
future merges must use ordinary Git merge behavior.

## Working views and builds

A hardware worktree can be focused with:
`git sparse-checkout set --cone .devcontainer .github docs scripts tests superfw`
(substitute its target). Root files remain visible automatically. Sparse settings
are worktree-local; `git sparse-checkout disable` restores the full view. Never
hand-delete tracked directories. Shared CI uses complete checkouts.

`make` prints help. `make ezfomegade`, `make ezfomega`, `make ezfblue`, `make superfw` explicitly
select a project; SuperFW additionally accepts `BOARD=sd|lite|chis`. `make build-all`/`make verify-all` select projects materialized
in the current working view; on full dssmm they select all four. For a hidden
project, expand the sparse view first. Outputs remain private to the project.
Run `make check-isolation`, `python3 tests/test-project-isolation.py` and
`python3 tests/test-project-merges.py` when changing this architecture. Root CI
is read-only; Pages publication remains manual/default-only.

projects.json is a common live registry, not independent frozen snapshots.
Its baseline commits record known kernel inputs. `scripts/check-snapshots.py`
checks those source/assets/build/test baselines (including sparse index inputs),
not live documentation. Differences during feature work are expected; this is
not a merge gate. Refresh root baseline references on trunk at validated release
checkpoints, never as a root edit in a hardware PR. Retain historical reports.

For the published 8.4.0 release, DE is stable/latest for Definitive Edition
Rev.A/B. Original OMEGA8.4.0 Test4 and BLUE8.4.1 Test4 remain hardware-untested prereleases. Builds, mocks and
static layout bounds do not qualify physical flash/save timing or runtime stack
peaks. No release changes are authorized merely by architecture maintenance.
Preserve credits, tags/assets, user edits and all journals. Use automatic GitHub
source archives, never upload extra source ZIPs. Candidate folders were retired.

Checkpoint before substantial shared changes and after milestones, recording
actual tests/results and exact continuation steps. Commit/push within the right
branch; verify remote state. Branch protection enforcement is a repository
setting, not a guarantee supplied by directory layout or CI.

Root README.md is a user-facing hardware/features/download/credits overview.
Keep developer commands, sparse/branch/merge procedures and architecture details
in docs/isolation.md and agent/journal files. Clearly distinguish the three OMEGA-family cards and the separate SuperCard
SuperFW project; retain the OMEGA/BLUE hardware-untested, caution/own-risk notice.
Keep credits positive and accurate; retain Merkin Mix's DS Style fork lineage.

For local VS Code/Docker/Ubuntu setup and session recovery, read
docs/local-development.md. Codespaces /tmp caches/process handles and ignored
binaries are not portable; use committed evidence and rerun unfinished checks.

MiGoomba full local verifiers, replay and focused Cppcheck completed on 2026-11-12;
see current HANDOFF and per-project migoomba-release-validation.json. All four hosted CI runs passed, and 8.4.0 is published
(DE stable/latest, OMEGA Test4/BLUE Test3 prereleases); downloaded assets match
the local files. See docs/releases/8.4.0-publication.json. New hardware qualification and old saved-state compatibility are not
claimed. Preserve user README commit e72b9be and all earlier published releases.

Release notes use four ordered changelog sections: New features, New fixes,
Summary of past features, Summary of past fixes. Keep cumulative summaries
concise. Fold the immediately preceding release's new features/fixes into the
appropriate past summary on the next release; do not leave them in additional
per-version sections. Preserve accurate model/hardware warnings, credits and validation limits. Keep
public notes focused on user-relevant changes; omit GitHub source-archive notices,
raw development-file warnings, asset byte counts and SHA-256 checksums (GitHub
shows downloadable assets and the repository retains verification evidence).

BLUE 8.4.1 Test4 is published after the FrankieT19 comparison and four passing
final CI runs; docs/releases/blue-test4-publication.json records the release and
owner-authorized condensed corrections to all three preceding8.4.0 note bodies.
BLUE uses a stock-size firmware-free image with full-image seal; use the official
BLUE kernel for FPGA updates. Persistent NOR RTS combinations remain blocked
pending a compatible validated runtime. Neither upstream Test5 nor our combined
port has confirmed physical BLUE validation. Read private BLUE guidance first.

GitHub already displays each release title/version. Do not repeat that build
heading in the note body. Apply the four-section style consistently to the full
release history while keeping changes accurate for each historical build; mark
undocumented fallback changes as undocumented, preserve model-specific warnings
and evidence, and never add later features to older binaries. Original bodies
are archived under docs/history/releases/ before a history-wide editorial pass.

## 2026-11-12 — Trim redundant release-page asset instructions

- Owner requests removing end-user-unneeded lines from release bodies, including GitHub's automatic source-archive notice, the packed `.bin` versus raw development `.gba` warning, and SHA-256 lines. Applied to all19 canonical notes and release bodies; removed redundant asset filenames/byte counts as well. Retained model compatibility, hardware-testing limits, save/update cautions, credits and the four ordered changelog sections.
- Exact remote bodies and metadata before this cleanup are archived at docs/history/releases/before-end-user-cleanup-2026-11-12.json. Update docs/releases/unified-release-notes-2026-11-12.json after publication with new body hashes/word counts; verify all19 remote bodies match canonical files and release metadata/assets remain unchanged. Documentation only; no kernel tests are needed.

## SuperFW project lane

The fourth project is the independent GPL SuperFW source for SuperCard GBA flash
carts and derivatives, pinned at upstream commit
`dabbf4909f85e556f7a12972cc8a7b94fc54ac51`. Its source, assets, licenses, tools,
tests and journals belong under `superfw/`; its development branch is `superfw`.
Build variants are `sd`, `lite` and `chis`. Do not combine its code or hardware
protocols with the OMEGA-family kernels. Read `superfw/AGENTS.md` and the project
current journals before SuperFW work. Local import is not hardware validation.

SuperFW bootstrap is complete: upstream snapshot dabbf4909f85e556f7a12972cc8a7b94fc54ac51 is isolated under `superfw/`, and five-branch CI passed on 2026-11-12. `superfw/docs/bootstrap-validation.json` records builds/tests and the explicit no-hardware-validation limit. The lane has no DSSMM release yet; never imply physical SuperCard qualification from hosted builds/tests.

SuperFW Test1 is published as prerelease superfw-test1, source8a22da16c2d755de8fc12db7c5482657e6813ecd, with three board-specific .fw assets. The owner reports successful chain-loading and manual themes on one SuperChis Prime v1.0 (SuperFW 0.22; stock kernel 0.20); this is an initial boot/theme smoke test only. SD/Lite, gameplay, saves, artwork, NOR and installation are not hardware-validated. The menu remains mostly upstream SuperFW; the full DSSMM-style layout is in progress. Read superfw/docs/prime-test1-hardware-observation.json and project journals. Preserve release/tag/assets and DEv8.4.0 latest stable. Root README is an end-user overview including artwork setup, not development instructions.

## 2026-11-13 — SuperFW Test 2 status

SuperFW Test 2 is published as the `superfw-test2` hardware-validation-pending prerelease for Chis, SD and Lite. Tag source is `c748fea2286721da21aa3394cf8d4ac330b19c8a`; all five hosted project checks and local board/package checks passed, and public assets match the verified builds. The new artwork default/settings and SYSTEM save/state behavior have no Test 2 hardware validation. See `superfw/docs/releases/superfw-test2-publication.json`; retain Test 1 and all earlier releases. DE `v8.4.0` remains latest stable. Clean ignored SuperFW build outputs after release verification and keep this in future handoffs.
