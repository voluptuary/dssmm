# DSSMM agent charter

Repository: <https://github.com/voluptuary/dssmm>
Default/integration branch: `dssmm`
Hardware lanes: `ezfomegade`, `ezfomega`, `ezfblue`, `superfw`
Retired branches: do not recreate `main` or `ezfode`.

Read this file first, then [`docs/AGENT-QUICKSTART.md`](docs/AGENT-QUICKSTART.md)
for commands, current-state pointers, evidence locations and search hygiene. Use
`HANDOFF.md` §7–§8 for the recent task log and open threads, and `PROGRESS.md`
for milestone history. Before touching a project, read that project's own
`AGENTS.md`; the per-project `HANDOFF.md`/`PROGRESS.md` are large append-only
journals, so grep their newest headings instead of reading them whole.

## 1. Mission

DSSMM ("DS Style Merkin's Mix") is a DS-style game-menu firmware family. One
shared root and one development process support four independent kernels:

| Directory | Card | Lane purpose |
| --- | --- | --- |
| `ezfomegade/` | EZ-FLASH OMEGA Definitive Edition Rev. A/B | Stable, primary lane |
| `ezfomega/` | Original EZ-FLASH OMEGA | Experimental port |
| `ezfblue/` | Newer EZ-FLASH OMEGA Definitive Edition B / BLUE | Experimental port |
| `superfw/` | SuperCard SD, Lite and SuperChis/Prime | Independent GPL SuperFW fork/port |

Goals, in order:

1. Keep the stable Definitive Edition firmware correct, buildable and
   reproducible.
2. Bring the Original OMEGA and BLUE ports to the same feature quality, then
   obtain physical qualification where possible.
3. Continue the SuperFW port toward DS Style menu/artwork/settings parity
   without pretending emulator or host evidence is cartridge qualification.
4. Preserve upstream licenses, credits, user data and every published artifact.
5. Publish only validated, owner-authorized releases.

Hardware status is not implied by a successful build. Only an explicit physical
observation from the owner (card, build, feature, result) qualifies hardware
behavior. Emulator fixtures, static layout bounds and CI prove software paths
only.

## 2. Operating parameters

- Work token-efficiently: reuse recorded evidence, batch independent checks,
  read only the relevant files, and do not rerun expensive suites for
  documentation-only changes.
- Verify before claiming. Running or interrupted jobs are not passes. If a
  command was lost, rerun it; never convert a missing log into evidence.
- Prefer the smallest correct change. Do not refactor unrelated code, reformat
  vendor files, or "fix" pre-existing warnings opportunistically.
- Keep changes inside the owning project directory. Only universal
  environment/build/CI/policy/docs/licensing belong at root.
- Never copy or symlink one card's driver/protocol into another project without
  explicit review. Card save, transfer, flash and reset protocols are separate.
- Preserve user edits (including the owner's README lineage), credits, tags,
  release assets, journals and ignored local caches. Never delete tracked history
  to "clean up".
- Do not add a GBA BIOS image, commercial ROM, generated firmware, emulator
  state or source archive to the repository or releases.
- Do not invent features, versions, hardware results or release dates.
- Record the actual test/result and exact continuation point at each checkpoint.
- Commit and push only within the correct branch; verify the remote state.

Definition of done for an implementation change: the owning project builds, its
documented verifier passes, new behavior has focused tests, no other project is
touched, evidence is recorded in the project, and the change is integrated
through the normal merge flow. Documentation-only changes need only their
relevant checks.

## 3. One tracked layout, separate hardware ownership

All five branches track the **same complete tree**: universal root
infrastructure plus `ezfomegade/`, `ezfomega/`, `ezfblue/` and `superfw/`.
Project branches are development lanes, not different repository layouts.
GitHub therefore shows all four projects on every branch. Never commit the
removal of other projects to "focus" a branch. On a hardware branch, edit only
its matching directory, including that project's own
`AGENTS.md`/`HANDOFF.md`/`PROGRESS.md`.

An optional worktree-local sparse checkout hides unrelated projects without
removing them from Git:

```sh
git sparse-checkout set --cone .devcontainer .github docs scripts tests ezfomega
make ezfomega
make verify-ezfomega
```

Substitute the lane's directory; include `superfw` for that lane. Root files
stay visible. `git sparse-checkout disable` restores the full view. Sparse
settings are local to the worktree. Never hand-delete tracked directories.

Root `AGENTS.md`/`HANDOFF.md`/`PROGRESS.md` cover shared architecture and
integration milestones only. Routine single-project work updates that project's
journals, not the root logs, to avoid cross-branch conflicts. Preserve every
historical log; pre-conversion branch records live under each project's
`docs/history/pre-shared-tree/`.

SuperFW is the independent GPL source derived from upstream commit
`dabbf4909f85e556f7a12972cc8a7b94fc54ac51`; its provenance/license is documented
in `superfw/docs/UPSTREAM.md`. Local import/compilation never establishes physical
SuperCard compatibility. Root README is a user-facing overview, not a developer
guide: keep the three OMEGA-family cards and the separate SuperFW project clearly
distinguished, retain the OMEGA/BLUE hardware-untested own-risk warning, and keep
credits positive and accurate (including Merkin's Mix's DS Style fork lineage).

## 4. Merge and integration workflow

1. Develop and test on the matching hardware branch, confined to its directory.
2. Merge that branch into `dssmm` normally (local merge or PR). CI compares
   changed paths against the merge base and rejects off-target/root edits.
3. Use merge commits for long-lived hardware PRs; do not squash or rebase them.
   Keep the hardware lanes after integration.
4. Merge `dssmm` back into hardware branches to synchronize the combined tree.
   Fast-forward when possible; never reset or force-push to sync.
5. Make universal root changes on `dssmm` or a shared feature branch, then sync
   them outward. Do not mix root changes into a hardware-to-`dssmm` PR.

Direct hardware-to-other-hardware PRs are rejected; route integration through
`dssmm`. Do not install blanket ours/theirs merge drivers or bypass failing
checks. Same-file edits can still conflict; resolve those explicitly. CI is
read-only and never pushes, merges or publishes; Pages publication is manual and
only from `dssmm`. Required-review enforcement is a repository setting, not a
guarantee supplied by directory layout.

`projects.json` is a live registry. Its `baseline_commit` values record known
kernel inputs for `scripts/check-snapshots.py`; they are not frozen
documentation and not a merge gate. Feature work may legitimately differ.
Refresh root baselines on `dssmm` only at validated release checkpoints.

## 5. Build and verify

Use the universal devcontainer; see `HANDOFF.md` for setup and recovery.

```sh
make                       # help
make ezfomegade            # build one project
make verify-ezfomegade     # build + project verifier
make ezfomega / make verify-ezfomega
make ezfblue  / make verify-ezfblue
make superfw BOARD=sd|lite|chis
make verify-superfw
make build-all             # all projects materialized in this view
make verify-all
make check-isolation
python3 tests/test-project-isolation.py
python3 tests/test-project-merges.py
```

SuperFW's three board variants share build intermediates: build them
sequentially, never in parallel. Generated `.elf`, `.gba`, `.bin` and `build*`
outputs are ignored; rebuild them locally. Build numbers derive from the full
Git history, so reproduce a recorded build number with a full clone and the
exact recorded source commit, not a shallow checkout.

When changing the shared layout/branch architecture, run the three isolation/
merge checks above. When changing release inputs, also run
`python3 scripts/check-snapshots.py`.

## 6. Release policy

Current published state (verify on GitHub before relying on it):

| Lane | Release | Tag/source | Status |
| --- | --- | --- | --- |
| DE Rev. A/B | 8.4.1 | `v8.4.1` / `a948b936` | Stable, latest |
| Original OMEGA | 8.4.1 Test 5 | `v8.4.1-omega-test5` / `29bbb055` | Prerelease, hardware-untested |
| BLUE | 8.4.1 Test 5 (firmware reports 8.4.2) | `v8.4.1-blue-test5` / `9bc24e04` | Prerelease, hardware-untested |
| SuperFW | Build 822 | `superfw-build822` / `2f73b53b` | Normal release; software-qualified only, Chis hardware-tested |

Build 822 is the current public and owner Chis hardware-test image. Build 827 is
a newer local software checkpoint and is not published.

Release rules:

- Publish only when the owner requests it, after the full local and hosted
  checks for that lane pass and downloaded assets match local bytes.
- Release bodies use four ordered sections: **New features**, **New fixes**,
  **Summary of past features**, **Summary of past fixes**. Fold the immediately
  preceding release's new items into the next release's summaries. Do not repeat
  the version heading that GitHub already displays.
- Keep public notes user-focused: model compatibility, real warnings, credits
  and validation limits. Omit GitHub source-archive notices, raw development-file
  warnings, asset filenames/byte counts and checksums.
- Mark undocumented historical changes as undocumented. Never attribute later
  features to an older binary. SuperFW Build 769 is the owner-approved exception
  that uses a cumulative fork catalog instead of delta/past sections; its
  canonical notes and publication record document that choice.
- Use automatic GitHub source archives; never upload another source ZIP or
  recreate candidate folders.
- Every kernel release must include the local `.release-assets/folder-art.zip`.
  Run `python3 scripts/prepare-folder-art.py /workspaces/dssmm/.release-assets/folder-art.zip --check`
  before upload, verify the downloaded asset, and preserve owner-added system
  icons. If the archive is missing, recover the exact asset from the latest
  published release or owner backup; never substitute a generated letter-only
  pack. Only `#`/`A–Z` Title images use centered, unscaled Box sprites on
  120x80 canvases; Box 80x80 images stay unchanged. Historical releases need no
  retroactive artwork change.
- Keep one public prerelease page for each of SuperFW, BLUE and Original OMEGA.
  Before publishing a newer prerelease, archive the superseded body and asset
  metadata under `docs/history/releases/`, then remove the old page/assets.
  Keep git tags. Never prune stable DE releases or their history.
- BLUE uses a stock-size, firmware-free image with a full-image seal and requires
  the official BLUE kernel for FPGA updates.
- After publishing, query GitHub `/repos/voluptuary/dssmm/releases/latest`.
  `--latest=false` on creation is not proof. If DE must remain latest, set
  `make_latest: true` through the REST API and verify the endpoint again.
- Do not recreate the withdrawn SuperFW Build 640/Test 6 release or retag
  published history. Its tag and publication evidence are historical records.

## 7. Roadmap

Current priorities:

1. Finish and integrate the SuperFW Build 827 shared-workspace/RAM change,
   then prove a fresh clone can build all four projects and all six board
   variants from the recorded sources.
2. Implement the owner-requested 19px SuperCard top bar and matching colors
   within the validated memory budget.
3. Continue SuperFW parity: settings persistence and layout, artwork/NOR
   carousel behavior, PCE icon/return polish, plugin exit paths, then SD/Lite
   behavior where hardware permits.
4. Keep Atari 2600 plugin development in <https://github.com/voluptuary/a26.gba>.
   DSSMM retains only historical recipes/provenance; do not resume the engine
   here.
5. Recover RAM headroom before adding substantial UI features. Do not move
   byte-parsed settings into cartridge SDRAM or reduce settings capacity or the
   stack reserve.
6. Maintain the EZ-FLASH lanes, fix regressions with focused fixtures, and take
   physical qualification opportunities when the owner offers them.
7. At release checkpoints, refresh only the affected project's input baseline,
   user-facing README links and canonical release notes.

Open risks: SuperFW main-EWRAM is near its limit; BLUE/OMEGA are physically
untested; SuperFW SD/Lite are not physically qualified; NOR RTS combinations
remain blocked pending a compatible validated runtime; preview GIFs and mGBA
fixtures are not performance measurements.

## 8. Do not

- Do not recreate `main`/`ezfode` or re-add removed release pages.
- Do not publish test builds, source ZIPs, BIOS files or generated binaries.
- Do not claim hardware validation from a build, mock, emulator or CI run.
- Do not combine OMEGA-family and SuperFW drivers/protocols.
- Do not add a public repository ZIP or Artifacts containing one.
- Do not bypass failing checks, force-push, or rewrite published tags/assets.
- Do not update the root logs for routine project work.
