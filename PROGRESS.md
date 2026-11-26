# DSSMM progress log

Purpose: a running record of milestones, dead ends, failed attempts, fixes,
workarounds, staged work and possible future tactics. Newest first. This is the
condensed root log; detailed per-project evidence lives in each project's
`docs/`, release JSON and git history.

The full root logs before the 2026-11-26 restructure are archived verbatim in
[`docs/history/root-journals-2026-11-26/`](docs/history/root-journals-2026-11-26/).
Earlier pre-shared-tree branch journals are under each project's
`docs/history/pre-shared-tree/`.

## 2026-11-26 — SuperFW Build 822 published

- Rebuilt all three boards from `2f73b53b`; sd/lite/chis hashes match
  `superfw/docs/build-822-validation.json` exactly. Published the normal release
  `superfw-build822` with the three `.fw` images and full `folder-art.zip`.
- Notes follow the Build 769 cumulative catalog with the 822 cartridge icons,
  DE-aligned settings rows, PCE return fix and external Atari status. Body
  matches the canonical file; all four downloaded assets are byte-identical.
- `--latest=false` again failed to preserve `/latest`: 822 became latest. DE
  `v8.4.1` was restored with REST `make_latest=true` and the endpoint verified.
  Build 827 remains an unpublished local software checkpoint.
- Evidence: `superfw/docs/releases/superfw-build822-publication.json`.

## 2026-11-26 — Root journal restructure

- Split the three overgrown root files by purpose: `AGENTS.md` (charter,
  architecture, release policy, roadmap), `HANDOFF.md` (cold-start rebuild and
  recent task state) and this milestone log. Removed duplicated release/CI
  narratives and stale per-checkpoint entries while preserving the full originals
  in `docs/history/root-journals-2026-11-26/`.
- No firmware, build, test or release content changed. Documentation-only
  change; no kernel tests were rerun.

## 2026-11-24/25 — SuperFW Build 822/827, RAM workspace, Atari externalized

- Build 822 (source `2f73b53b`) passed full all-3 board, native ARM, host, mGBA
  and IGM checks plus source/integration CI; recorded in
  `hardware-test/build822.json`. It is the owner's current Chis hardware-test
  image. Owner reported on Chis: Boots, Atari games load and Exit works, PCE
  games load and (later) PCE Exit works; PCE icon remains generic. Exact plugin
  revision was not stated.
- Main EWRAM was nearly full (Build 822: 256,648 of 257,024 bytes). Build 827
  (`db4e640`) added `menu_workspace.c/.h`, a strictly leased union of the 9,600 B
  selected-art hot copy and 8,192 B settings snapshot. Settings parsing/commit/
  readback/rollback hold an explicit lease; generation invalidates stale art.
  Five SDRAM artwork slots, 8,192 B settings capacity and the 16 KiB stack floor
  remain. Development Chis link recovered about 8,144 B (248,504 B EWRAM used);
  all-3 host/FAT16/FAT32, ARM cart-bus, linked save/reload and simultaneous
  native save/art checks passed.
- The first local full verifier stopped on a new workspace test's fixed 5 s
  marker. The failure was retained, focused reruns passed, and the fixture was
  changed to bounded completion polling with a distinct pending color. Kernel
  inputs and bytes were unchanged by that fixture fix.
- Atari 2600 plugin development moved to <https://github.com/voluptuary/a26.gba>.
  The owner upload is preserved only as provenance metadata in project history;
  the obsolete root/project ZIP was removed with owner authorization because it
  failed the isolation gate. No Atari engine changes remain in DSSMM.
- Public SuperFW is still Build 769. The superfw lane is ahead of `dssmm` until
  the normal integration/sync; do not publish 822/827 by accident.

## 2026-11-24 — EZ-FLASH SD-launch corrective releases

- Published DE `v8.4.1` (`a948b936`, stable/latest), Original OMEGA
  `v8.4.1-omega-test5` (`29bbb055`) and BLUE `v8.4.1-blue-test5` (`9bc24e0`,
  firmware still reports 8.4.2). Each passed its full local/source and all-4 CI.
- Reproduced and fixed with real native mGBA launch fixtures: DE/BLUE optional
  SD alias handling and Original OMEGA clipped-name handling. Card save,
  transfer and reset protocols were untouched.
- All six downloaded assets matched, including `folder-art.zip`. BLUE kept its
  sealed stock-size, firmware-free image and official-FPGA-update requirement.
  No affected-card physical confirmation; OMEGA/BLUE remain untested.
- Prior OMEGA/BLUE prerelease pages were removed only after exact archive and
  replacement download verification; tags retained. Archive:
  `docs/history/releases/prerelease-retention-sd-launch-2026-11-24.json`.
  DE latest endpoint was explicitly re-verified.

## 2026-11-24 — README showcases, canonical name, fork reproducibility

- Current project name is "DS Style Merkin's Mix"; historical journals and
  release bodies remain records of their time. BLUE's published relabel is
  `v8.4.1-blue-test5`; same source/assets.
- Root README gained four real native menu GIFs plus original fictional cover/
  title art under `docs/media/demo-art/` with prompts/provenance. Capture uses
  stock SDL mGBA 0.10.1, private Xvfb/XDG_CONFIG_HOME, xdotool and Pillow 9.4;
  no BIOS, emulator patch or new kernel driver. Captures are visual references,
  not hardware/timing proof.
- A fresh fork must reproduce the complete build/test state: the devcontainer
  feature lock is committed, all six board builds and checkpoint reproduction are
  documented in `docs/current-builds.md`, and the current SuperFW source was
  integrated through a normal hardware PR. Build numbers need full history.

## 2026-11-23 — SuperFW Builds 759/769 and BLUE launch audit

- SuperFW `superfw-build759` (`67b20b57`) and `superfw-build769` (`37c9e06`)
  were published as normal releases after all-3 local/native ARM/mGBA and all-4
  integration CI passed, with all downloaded assets and full `folder-art.zip`
  matching. Only prior Chis physical observations qualify hardware; PCE and
  SD/Lite remained open.
- BLUE SD audit fixed alias-free exFAT rejection with a permanent five-case
  native mGBA launch fixture. No BLUE release or physical claim.
- Release lesson: `--latest=false` on creation did **not** preserve GitHub's
  `/latest`; the build briefly became latest. It was restored by an explicit
  REST `make_latest: true` on the DE release and re-verified. Always query the
  endpoint after publishing.
- Owner later confirmed PCE games load on Chis 769 but Exit restarted the game
  and the PCE icon was generic; the current release note/README were corrected
  and the prior body archived. Firmware/assets/tags unchanged.

## 2026-11-17/18 — SuperFW Build 667; settings crash corrected

- Build 640/Test 6 hard-froze when changing Interface settings, so the owner
  withdrew it. The tag and publication evidence are retained; the build must
  never be recreated or retagged. Public downloads were temporarily unavailable.
- Root cause: GCC 16.1.0 ARM Thumb-1 IPA register allocation let a callee clobber
  a register its caller treated as preserved, specifically on the settings path.
  SuperFW now disables IPA-RA locally; no other kernel's compiler flags or
  hardware protocols changed.
- The fix used settings save-on-exit with cooperative verified steps, independent
  header color and aligned clocks. A permanent native linked-ARM fixture
  (source/generator/runner/docs tracked on the default branch) reproduces the
  class of fault that host-only tests missed.
- Build 667 (`a7259a83`) was published as a normal release after all-3 native/
  IGM/ARM/mGBA and all-4 CI passed. Only Chis has owner hardware observations;
  new timing/gameplay and SD/Lite remain unqualified. GG/SMS Exit, PCE and full
  DE parity remained open.

## 2026-11-16 — SuperFW Test 5, Build 624, DMA replay

- Test 5 (`a4cafca`, Build 624) published with Chis/SD/Lite images and full
  `folder-art.zip`; all local/native/IGM/ARM and integration CI passed. Test 4's
  page/assets were archived then removed; its tag retained.
- Added optional pixel-exact SuperFW DMA replay using stock SDL mGBA 0.10.1,
  Xvfb and the shared standard-library PNG reader: 12 Light/Dark background
  frames, every 38,400 pixel checked plus framebuffer guards. No emulator patch,
  new dependency or private BIOS. Fixture cycle counters are not physical FPS.
- Test 6/Build 640 followed, then was withdrawn for the settings freeze above.

## 2026-11-14 — SuperFW Test 4 and prerelease retention policy

- Test 4 (`19429735`, Build 531) published for all three boards. Title mode
  checks original EZ-FLASH GBA game-code artwork after filename matching; Box
  mode remains DSSCC filename-matched. All hosted/local/package/download checks
  passed; no hardware validation claimed.
- Owner set the retention policy: keep one public prerelease page per SuperFW,
  BLUE and Original OMEGA lane; archive superseded bodies/asset metadata under
  `docs/history/releases/` before removal; retain tags; never prune stable DE.
- Added the standard mGBA preview environment (stock 0.10.1 at
  `/usr/games/mgba`, Xvfb/xdotool, dummy audio) and corrected the packaged
  preview. Chis artwork/boot observations do not qualify plugin timings.

## 2026-11-13 — SuperFW Test 2/Test 3

- Test 2 (`c748fea`) published for Chis/SD/Lite, then superseded. Test 3 followed
  the cross-variant port work. Artwork/settings and SYSTEM save behavior were not
  hardware-validated at that stage.

## 2026-11-12 — SuperFW lane, MiGoomba 8.4.0, release-note unification

- SuperFW bootstrapped as the fourth isolated project from upstream
  `dabbf49`, with its own GPL notices, tests, assets and `sd`/`lite`/`chis`
  variants. A first clean-checkout CI failure (six upstream emulator payloads
  omitted by ignore rules) was fixed by tracking those exact assets and adding a
  narrow ignore exception.
- MiGoomba v0.1 (`4b401e2`) was embedded per platform (`ezode` for DE, `ezo` for
  Original, `generic` for BLUE to avoid unverified DE rumble writes). Generated
  headers are committed, so normal builds need no upstream download.
- Published 8.4.0: DE stable/latest, Original OMEGA Test 4 and BLUE Test 3
  prereleases. All local verifiers, save/conversion/legacy replays, focused
  Cppcheck, packaging, asset downloads and hosted CI passed. No new physical or
  old-save compatibility claim.
- Unified all 19 release bodies to the four ordered sections (about 43% shorter),
  preserving model warnings, credits and historical limits. Original bodies and
  metadata were archived before editing.
- BLUE Test 4 followed the FrankieT19 comparison: stock-size firmware-free
  sealed image, DDR state loading, pending-save/NOR/map/patch/settings
  safeguards, and persistent NOR RTS combinations explicitly blocked until a
  compatible validated runtime exists. Physical validation remains outstanding.
- Codespaces-to-local migration: documented one-clone/one-container recovery in
  `docs/local-development.md`. Lost `/tmp` logs and process handles were treated
  as unverified and rerun locally.

## 2026-11-11/12 — Shared-tree compartmentalization

- Replaced the old per-branch layouts with one identical combined tracked tree on
  all branches. Hardware branches became development lanes; worktree-local sparse
  checkout hides unrelated projects without deleting them from Git.
- Preserved all pre-conversion journals under each project's
  `docs/history/pre-shared-tree/`. A one-time ancestry bridge recorded the old
  already-imported hardware heads without rewriting history; future merges use
  ordinary Git.
- Added CI changed-path scope checks, reverse `dssmm`→lane sync support, sparse-
  aware layout/build selection, a live `projects.json` registry and real-Git
  isolation/merge fixtures.
- Corrected layout misunderstandings: projects are direct root children (no
  `targets/` wrapper); `dssmm` contains all four; hardware lanes use the same
  matching directory plus universal root.
- Verified all builds, sparse verifiers, merge rehearsals, release-byte
  comparisons (DE needed an explicit `GAME_TITLE` to reproduce its published
  header) and the user-facing READMEs.

## 2026-10-16 → 2026-11-10 — Adversarial audit and 8.x hardening

Condensed from the archived root logs; detailed private evidence is in the
EZ-FLASH projects' `docs/` and historical release records.

- NOR work: F-022 status/error propagation, IWRAM relocation, full readback and
  device-protocol comparison; S71 metadata IRQ/VBlank hazards corrected.
- Loader work: PSRAM load policy bounded reads, exact-read/close handling,
  chunk planning, 8 MiB ROM and legacy 4 MiB plugin window handling, abort before
  patch/boot. No further code-level defect was found; physical selector behavior
  stayed unverified.
- Memory work: RAM/stack/cache audits, heap bounds, cache invalidation and layout
  guards; EWRAM/IWRAM margins recorded. Static bounds never qualified runtime
  stack peaks.
- Save/settings work: transactional replacement, long-LFN backup path, CP936
  encoding, marked-art correction, save-path rotation and manual-import
  compatibility.
- ROM-size audit: lossless NOR asset packing enabled the 8.3.1 refresh; refreshed
  releases were downloaded and verified.
- Releases from 7.4.x through 8.3.2 were published; DE's published hash needed
  an explicit `GAME_TITLE` because `gbafix` otherwise derived the header title
  from the checkout directory name.
- Dead ends/lessons: a 2 MiB-capped candidate produced owner-reported white
  screens; save-popup artwork alignment remained unresolved on hardware; GitHub
  default-branch rename attempts returned 403 until the owner acted; ASan host
  runs looped under PIE until harness binaries were rebuilt with
  `-fno-pie -no-pie`.

## Staged items and future tactics

- SuperFW: finish Build 827 integration, then the 19px SuperCard top bar/colors.
  Recover RAM only with explicit ownership/lease rules; never move byte-parsed
  settings into cartridge SDRAM or shrink settings/stack budgets.
- SuperFW parity still open: PCE icon/return polish, GG/SMS/plugin Exit paths,
  settings persistence/layout, NOR carousel edges, SD/Lite behavior.
- Optional SuperFW card-bus mGBA harness improvements exist in review notes; they
  are a future testing extension, not a replacement for native mocks or physical
  qualification. Do not combine them with EZ drivers.
- Atari 2600 work belongs in `a26.gba`; DSSMM keeps only provenance.
- EZ-FLASH: pursue BLUE updater/FPGA verification and Original OMEGA hardware
  coverage when the owner can test. Keep save/transfer/reset protocols separate.
- Release engineering: always verify `/releases/latest`, archive before pruning,
  and check downloaded assets against local bytes.
- Reproducibility: use full clones and recorded source commits for build numbers;
  rerun any interrupted check rather than trusting a missing log.
