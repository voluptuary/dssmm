# DSSMM shared-tree working architecture

Always work in the most token-efficient manner possible: reuse existing findings and validation evidence, read only relevant context, batch independent checks, and avoid duplicate work or unnecessary output. Preserve correctness, required testing, and clear continuation notes.

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
The standard devcontainer includes mGBA SDL plus Xvfb/xdotool for GBA preview
testing. Keep any BIOS image outside the repository/releases; document emulator
version, package, launch-flag or harness changes in docs/local-development.md
and the root handoff/progress logs.
SuperFW's optional pixel-exact DMA replay is documented in
`superfw/docs/BACKGROUND-PERFORMANCE.md`: stock SDL mGBA0.10.1/Xvfb, no emulator
patches/new dependencies/private BIOS required. Keep the shared screenshot
reader and actual pass/fail evidence in the project when changing its harness.

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

SuperFW Test1 was originally published as prerelease `superfw-test1`, source `8a22da16c2d755de8fc12db7c5482657e6813ecd`, with three board-specific .fw assets. Its release page/assets were removed after Test 4 under the owner’s one-prerelease-per-lane policy; the git tag and publication record remain. The owner reports Test 1 chain-loading and manual themes on one SuperChis Prime v1.0 (SuperFW 0.22; stock kernel 0.20); this is an initial boot/theme smoke test only. SD/Lite, gameplay, saves, artwork, NOR and installation are not hardware-validated. Read `superfw/docs/prime-test1-hardware-observation.json` and project journals. DE `v8.4.0` remains latest stable. Root README is an end-user overview including artwork setup, not development instructions.

## 2026-11-13 — SuperFW Test 2 status

SuperFW Test 2 (`superfw-test2`, source `c748fea2286721da21aa3394cf8d4ac330b19c8a`) was a hardware-validation-pending prerelease for Chis, SD and Lite. Its release page/assets were removed after Test 4 under the owner’s retention policy; its git tag, canonical notes and publication record remain. Test 2 artwork/settings and SYSTEM save/state behavior were not hardware-validated. See `superfw/docs/releases/superfw-test2-publication.json`. DE `v8.4.0` remains latest stable. Clean ignored SuperFW build outputs after release verification.

## 2026-11-14 — SuperFW Test 4 current release

SuperFW Test 4 is the current three-board hardware-validation-pending prerelease, tag source `19429735b3b55f7c57dd3921f0aad61601521e06` (DSSMM Build 531). Title mode now checks original EZ-FLASH GBA game-code artwork after filename matches; Box remains DSSCC filename matched. All five hosted checks, local artwork/settings tests, three board builds, package integrity checks and byte-identical download checks passed. No Test 4 hardware validation is claimed. See `superfw/docs/releases/superfw-test4-publication.json`. Per owner request, keep only the newest prerelease page for SuperFW, BLUE and Original OMEGA; older release bodies/assets are archived at `docs/history/releases/prerelease-retention-2026-11-14.json`, and tags remain. DE `v8.4.0` and its release history are unchanged. Root README links to Test 4.


Release retention policy: when a new prerelease is published for SuperFW, BLUE or Original OMEGA, remove older prerelease release pages/assets in those three lanes after preserving their exact bodies/asset metadata in `docs/history/releases/`. Retain git tags. Keep only each lane newest prerelease; do not remove stable DE or its history.


Keep one public prerelease release page per SuperFW, BLUE and Original OMEGA lane. On each new prerelease publication, first archive superseded release bodies and asset metadata under `docs/history/releases/`, then remove the older release pages/assets while retaining git tags. Never prune stable DE releases or assets.

## Shared folder artwork release asset

Owner requires local `.release-assets/folder-art.zip` on every future kernel release,
for all four project lanes. Include it alongside firmware assets; this is artwork,
not a source archive. Run `python3 scripts/prepare-folder-art.py /workspaces/dssmm/.release-assets/folder-art.zip
--check` before upload and verify the downloaded release asset matches. Preserve
owner-added system icons. Only #/A–Z Title images use centered unscaled Box
sprites on120x80 canvases; Box80x80 images remain unchanged. Historical releases
need no retroactive change. The owner removed the public repository ZIP; do not
re-add it. Preserve this ignored local archive during cleanup. Root release-policy
maintenance belongs on dssmm. If the local archive is missing in a later session,
recover the exact asset from the latest published release or owner backup before
publishing; never substitute the smaller generated letter-only pack.

## 2026-11-16 — SuperFW Test5 current prerelease

SuperFW Test5 is published at sourcea4cafcaf1da9539234c01de0a6933d0816371698,
DSSMM Build624, with Chis/SD/Lite images and retained full folder-art.zip.
All3/full local/nativeIGM ARM checks, hardwareCI43868556156 and all4 integration
CI43868982790 pass; downloaded assets match. See project publication/validation
records. Test4 release page/assets were removed after exact body/asset archive
at docs/history/releases/prerelease-retention-2026-11-16-test5.json; git tag kept.
BLUE/OMEGA newest prereleases and stable DEv8.4.0 remain unchanged. Earlier Chis
artwork/boot observations do not qualify new plugin timings, Exit, full settings
persistence, SD/Lite physical operation or complete DE parity. Owner requests
adversarial SuperFW audit next, then continued autonomous porting; keep624 frozen.

## SuperFW Test6 integration/release checkpoint

Current SuperFW prerelease is superfw-test6, Build640/source4a85efb8, with all3
firmware assets and full folder-art.zip verified by download. HardwareCI43978497350,
all4 integrationCI43979085712 and full local/nativeIGM checks passed. Exact prior
Test5 metadata archived at docs/history/releases/prerelease-retention-test6.json
before its release page/assets were removed; tags retained. Other lanes and
stable DEv8.4.0latest unchanged. Publication record is under superfw/docs/releases/.
Root README links/registry baseline refreshed at this validated release checkpoint.
No new physical qualification:640 settings persistence/PCE/plugin Exit/NOR/full
DE parity remain unconfirmed/incomplete. See project journals; continue work there.

URGENT: owner withdrew/deleted SuperFW Test6 after Build640 hard-freezes when
changing Interface settings. Do NOT recreate640/Test6. Preserve tag and historic
publication evidence; public downloads are temporarily unavailable. Root README
records withdrawal; project crash investigation takes precedence over porting.
Host/mGBA/CI passes did not qualify this physical save/settings path. Next release
must use a new tag after corrective validation; do not retag published history.

Current corrective checkpoint: SuperFW PR9 merged after all4 CI/nativeARM/mGBA
save/restart checks passed. Chis647/sourcea55f4a7 is the local hardware-test; Test6/
640 remains withdrawn and must not be recreated. Settings save on exit. Native
fixture source/generator/runner/docs are tracked on dssmm and standard CI/container
supply dependencies. See superfw/docs/SETTINGS-ARM-REGRESSION.md and projectjournals.
Only SuperFW compilerflags changed; sharedenvironment changes are separate.


2026-11-17 — SuperFW Build667 normal release published at tag `superfw-build667`,
source a7259a83, after full all3/nativeIGM/ARM/mGBA18 and all4 PR12 checks passed.
PR12 merged b7d91488; permanent native fixture remains on the default repository.
Four downloaded assets and canonical notes match, including full folder-art.zip.
Only Chis has owner hardware tests; new667 timing/gameplay and SD/Lite remain
unqualified. Settings use cooperative verified steps, independent header color
and aligned clocks. GG/SMS Exit, PCE and full DE parity remain open. Project report:
superfw/docs/releases/superfw-build667-publication.json. Owner requested normal
release, not prerelease; DE v8.4.0 remains latest. No old SuperFW pages existed to
prune; BLUE/OMEGA each retain one prerelease, all tags/stable DE history preserved.
Updated user README and refreshed only SuperFW's validated input baseline.
Normal merge/sync follows this publication checkpoint; no other kernel inputs
changed. Chis667 is current ignored hardware-test image; owner lock/private BIOS,
session scratch and full artwork archive are retained. Project journals govern
further single-project work; recover RAM headroom before substantial additions.

Release latest verification: query GitHub `/repos/voluptuary/dssmm/releases/latest`
after publishing; `--latest=false` alone is not proof. Preserve DEv8.4.0 as
latest unless the owner changes that policy; explicitly set its `make_latest`
to `true` via REST if needed, then verify the endpoint again. Build769 follows
the owner's cumulative SuperFW-fork catalog request instead of delta/past
sections; its canonical notes and publication record document the exception.

## 2026-11-24 — EZ SD-launch corrective releases

DEv8.4.1/sourcea948b936 stable/latest, Original OMEGAv8.4.1-omega-test5/source29bbb05
and BLUEv8.4.2-blue-test5/source9bc24e0 prereleases published after each full
local/source/all4 PR23/25/24 CI passed. DE/BLUE optional-alias failure and OMEGA
clipped-name failure reproduced/fixed with actual native mGBA launch fixtures;
card save/transfer/reset protocols remain separate and unchanged. All6 downloaded
assets match, including full folder-art.zip on each; canonical four-section notes
match. BLUE keeps sealed stock2124252B format/official FPGA requirement.
No affected-card physical confirmation; Original/BLUE remain hardware-untested.
DEv8.4.1 latest endpoint explicitly verified, old DE stable history retained.
Prior OMEGA/BLUE prerelease pages/assets removed only after exact archive and
replacement download verification; git tags retained. Root retention archive:
docs/history/releases/prerelease-retention-sd-launch-2026-11-24.json.
Refreshed only these3 validated input baselines and end-user download links.
Normal hardware PR merges and publication-doc merges preserve lane ancestry;
normal all5 synchronization next. Project journals retain detailed evidence.
SuperFW777 pending clean local gate in its own lane;769 public unchanged.
