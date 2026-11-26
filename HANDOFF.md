# DSSMM cold-start handoff

Purpose: let an agent who knows nothing about this workspace rebuild it, verify
it, and resume the last task after an abrupt stop. Read `AGENTS.md` first for
mission/rules, then this file, then `PROGRESS.md` and the owning project's
`AGENTS.md`/`HANDOFF.md`/`PROGRESS.md`.

If you only need the current continuation, jump to
[§7](#7-current-state-and-recent-task-log) and [§8](#8-immediate-next-steps).

## 1. What this repository is

DSSMM is a DS-style menu firmware family in one shared tree:

| Directory | Hardware | Installable output |
| --- | --- | --- |
| `ezfomegade/` | EZ-FLASH OMEGA Definitive Edition Rev. A/B | `ezkernelnew.bin` |
| `ezfomega/` | Original EZ-FLASH OMEGA | `ezkernel.bin` |
| `ezfblue/` | Newer EZ-FLASH OMEGA DE B / BLUE | `ezbluekernel.bin` |
| `superfw/` | SuperCard SD / Lite / SuperChis | `build/<board>/superfw.fw` |

The first three are the OMEGA-family kernels and are **not interchangeable**.
SuperFW is an independent GPL fork for SuperCard-family carts. Root contains only
universal environment, build dispatch, CI, policy, docs, licensing and shared
journals.

Published state (re-verify on GitHub; see `docs/current-builds.md`):

- DE `v8.4.1` — stable/latest.
- Original OMEGA `v8.4.1-omega-test5` — prerelease, hardware-untested.
- BLUE `v8.4.1-blue-test5` (firmware reports 8.4.2) — prerelease,
  hardware-untested; sealed stock-size image, official FPGA update required.
- SuperFW public `superfw-build769`.

Local SuperFW Build 822 is the owner's current Chis hardware-test image; Build
827 is the current software checkpoint. Neither is published.

## 2. Rebuild from zero

### 2.1 Prerequisites

- Git.
- Docker (Docker Desktop with WSL2 integration on Windows, or Docker Engine on
  Ubuntu). Keep the clone in the Linux filesystem (`~/src`, not `/mnt/c`).
- VS Code with the Dev Containers extension. On Windows also install the WSL
  extension.
- No host devkitPro, Python packages or emulator installation is required for the
  container path.

### 2.2 Clone

```sh
mkdir -p ~/src && cd ~/src
git clone https://github.com/voluptuary/dssmm.git
cd dssmm
code .
```

Use a **full clone**, not a shallow checkout: SuperFW's build number is derived
from `git rev-list --count HEAD`.

Then run **Dev Containers: Reopen in Container** from the VS Code command
palette. The first build downloads `devkitpro/devkitarm:20260610` and the feature
layers; allow it to finish, then open a new terminal inside the container.

The image supplies devkitARM/libgba/Maxmod/GBA rules, GCC/G++/Make, CMake,
Cppcheck, lcov, Git/GitHub CLI, Python 3 with Pillow/Unicorn/PyYAML/setuptools,
Node 24, zlib/OpenSSL headers, and `mgba-sdl`, `xvfb`, `xauth`, `xdotool`.
`DEVKITPRO=/opt/devkitpro`, `DEVKITARM=/opt/devkitpro/devkitARM`.

### 2.3 If the devcontainer is corrupt or will not start

Work through these in order; none require deleting the repository:

1. Check the config is not locally damaged:
   ```sh
   git status --short .devcontainer
   git restore .devcontainer   # only if you see unintended local edits
   ```
2. In VS Code run **Dev Containers: Rebuild Container**. If it still fails, run
   **Dev Containers: Rebuild Container Without Cache**.
3. If the container record itself is broken, remove just that container and let
   VS Code recreate it:
   ```sh
   docker ps -a
   docker rm -f <dssmm-container-name>
   ```
   Do not delete the workspace folder, the bind-mounted clone, or unrelated
   containers/volumes.
4. Check Docker itself:
   ```sh
   docker version
   docker run --rm hello-world
   ```
   On Windows/WSL restart Docker Desktop and run `wsl --shutdown`, then reopen
   Ubuntu. On Ubuntu ensure the daemon is running (`systemctl status docker`).
5. If the base image will not pull or disk is full, free space carefully
   (`docker system df`, then remove clearly unused images/build cache) and pull:
   ```sh
   docker pull devkitpro/devkitarm:20260610
   ```
6. If feature downloads fail, rebuild while online; `devcontainer-lock.json` is
   tracked and pins the feature versions. Do not hand-edit it.
7. Last resort: native Ubuntu toolchain (no container). Install the devkitPro
   GBA toolchain and host dependencies, then set the environment:
   ```sh
   sudo apt-get update
   sudo apt-get install -y build-essential git gh make cmake cppcheck lcov \
     python3 python3-pil python3-unicorn python3-yaml python3-setuptools nodejs \
     zlib1g-dev libssl-dev mgba-sdl xvfb xauth xdotool
   # devkitPro pacman (see https://devkitpro.org/wiki/Getting_Started):
   # install devkitpro-pacman, then:
   sudo dkp-pacman -S gba-dev
   export DEVKITPRO=/opt/devkitpro
   export DEVKITARM=$DEVKITPRO/devkitARM
   export PATH=$DEVKITARM/bin:$DEVKITPRO/tools/bin:$PATH
   ```
   Verify `arm-none-eabi-gcc --version`, then run the build commands below.

The standard container is documented in `docs/local-development.md`; keep that
file, `.devcontainer/Dockerfile` and this handoff consistent when changing the
environment. Never add a GBA BIOS to the image, repository or releases.

### 2.4 Recover ignored/local assets after a workspace loss

Re-cloning restores all tracked sources, tools and native fixtures. These local
items are intentionally ignored and must be recovered separately:

- `.release-assets/folder-art.zip` — required for the next kernel release.
  Recover the exact archive from the latest published release or the owner's
  backup, then verify:
  ```sh
  python3 scripts/prepare-folder-art.py .release-assets/folder-art.zip --check
  ```
  Never substitute a generated letter-only pack.
- `hardware-test/` images — local owner-test artifacts; rebuild from the source
  commit in the matching `validation.json` if needed. Do not assume they are
  tracked.
- `/tmp/dssmm-*` preview worktrees, `/tmp` logs and generated `.elf`/`.gba`/
  `.bin`/`build*` files are disposable. Do not treat an old log as evidence.
- A private BIOS for local experiments stays outside the checkout and is removed
  afterwards.

## 3. Verify the checkout

Run inside the container from the repository root:

```sh
git status --short --branch
make                          # help
make check-isolation
python3 tests/test-project-isolation.py
python3 tests/test-project-merges.py

make ezfomegade && make verify-ezfomegade
make ezfomega   && make verify-ezfomega
make ezfblue    && make verify-ezfblue
make superfw BOARD=sd
make superfw BOARD=lite
make superfw BOARD=chis
make verify-superfw
```

`make build-all` / `make verify-all` select the projects materialized in the
current working view (all four on a full `dssmm` checkout). In a sparse lane view
they select only that lane. Build SuperFW board variants **sequentially**: they
share intermediates and overlapping builds corrupt the result.

Expected: verifiers exit 0, build inside each project, and SuperFW writes
`superfw/build/<board>/superfw.fw` plus `build-info.json`. Existing SDK/vendor
warnings are not new failures; compare against the recorded evidence.

Optional preview/capture smoke checks (stock mGBA 0.10.1):

```sh
export MGBA_BIN=/usr/games/mgba   # if not on PATH
bash superfw/scripts/test-browser-preview-mgba.sh
bash superfw/scripts/test-background-mgba.sh
```

mGBA does not emulate SuperCard SDRAM/SD protocols; a passing preview is not
cartridge or performance qualification. See `superfw/docs/BACKGROUND-PERFORMANCE.md`.

## 4. Branch and worktree workflow

Branches: `dssmm` (integration/default), `ezfomegade`, `ezfomega`, `ezfblue`,
`superfw`. All track the same combined tree. Develop only in the matching
project directory on its lane. Never force-push, reset, or delete other
projects to focus a lane.

One clone with branch switching is enough:

```sh
git switch ezfomegade
git sparse-checkout set --cone .devcontainer .github docs scripts tests ezfomegade
make verify-ezfomegade

git sparse-checkout disable
git switch dssmm
```

To integrate: merge/PR the lane into `dssmm` with an ordinary merge commit, then
merge `dssmm` back into the lane to sync. Use merge commits, not squash/rebase,
for long-lived hardware PRs. Direct hardware-to-other-hardware PRs are rejected.
See `docs/isolation.md` for the full policy.

Newer worktrees may exist under `/tmp` (preview/test fixtures). They are not
portable and not required; do not rely on their absolute paths.

## 5. Where the evidence lives

- `docs/current-builds.md` — current validated checkpoints and outputs.
- `docs/local-development.md` — environment, mGBA/Xvfb, capture tools.
- `docs/isolation.md` — layout, sparse views, merge policy, CI scope.
- `docs/releases/` — canonical release notes index and publication evidence.
- `docs/history/releases/` — archived bodies/asset metadata before prunes.
- `superfw/docs/` — Build 822/827 validation, settings/ARM regression,
  background/performance, artwork and porting plans.
- `ezfomegade/docs/`, `ezfomega/docs/`, `ezfblue/docs/` — per-card release,
  native-launch and validation records.
- `projects.json` — live registry and release-input baselines.
- `docs/history/root-journals-2026-11-26/` — full pre-restructure root logs.

## 6. Known environment gotchas

- Debian installs mGBA at `/usr/games/mgba`, which may be absent from `PATH`.
  Check `/usr/games/mgba --version`; the tested package is 0.10.1.
- Headless preview needs `SDL_AUDIODRIVER=dummy` and `xvfb-run -a`.
- Do not add a BIOS. Blank mGBA frames are usually an emulator/BIOS/startup
  configuration issue, not a render pass.
- Local `lcov` coverage may be unavailable; hosted CI runs the complete upstream
  suite. Do not count a skipped local coverage run as a pass.
- ASan host harnesses may need `-fno-pie -no-pie` on this Linux setup; the
  project harness already handles this where required.
- `git diff branch directory` can be ambiguous when a branch and directory share
  a name; use `git diff branch -- directory`.
- Snapshot baseline differences during feature work are expected and are not a
  merge gate.
- Interrupted or lost `/tmp` logs are unverified. Rerun the check.

## 7. Current state and recent task log

Newest first. Detailed entries are in `PROGRESS.md`; this is the short takeover
log.

### 2026-11-26 — root journal restructure

- The three overgrown root documents were split by purpose and their full
  originals archived under `docs/history/root-journals-2026-11-26/`. No firmware,
  build or release change. This restructure is a universal root change and
  belongs on `dssmm`, then syncs outward.

### 2026-11-24/25 — SuperFW Build 827 RAM workspace; Build 822 hardware test

- `superfw` lane: Build 827 (`db4e640`) added `menu_workspace.c/.h`, a leased
  union of the 9,600 B artwork hot copy and 8,192 B settings snapshot, recovering
  about 8,144 B of main EWRAM while preserving slot/capacity/stack limits.
  Focused host/FAT16/FAT32/ARM/linked/native checks pass; the first full verifier
  stopped on a fixed 5 s fixture marker, which was replaced with bounded polling.
  Evidence: `superfw/docs/build-827-validation.json`.
- Build 822 (`2f73b53b`) is the current Chis hardware-test image:
  `hardware-test/superfw-chis-build822.fw` / `hardware-test/build822.json`.
  Owner observations on Chis: boots; Atari games load and Exit works; PCE games
  load and PCE Exit now works; PCE icon remains generic. Exact plugin revision
  was not stated. No SD/Lite or full Atari qualification.
- Atari 2600 plugin development moved to <https://github.com/voluptuary/a26.gba>.
  The owner upload remains only as provenance metadata; the obsolete ZIP was
  removed with authorization.
- The superfw lane is ahead of `dssmm` and public SuperFW is still Build 769.
  Do not publish 822/827; integrate through the normal flow after gates pass.

### 2026-11-24 — SD-launch corrective releases and README

- Published DE 8.4.1, OMEGA 8.4.1 Test 5 and BLUE 8.4.1 Test 5 after the
  optional-SD-alias/clipped-name fixes were reproduced with native launch
  fixtures. All assets matched; DE latest verified; OMEGA/BLUE hardware remains
  untested.
- Root README gained real native menu GIFs and original sample art. Capture uses
  stock mGBA/Xvfb/xdotool/Pillow; these are documentation references only.
- Fork reproducibility: devcontainer feature lock committed, all six board builds
  documented, full-clone requirement recorded.

### 2026-11-23 — SuperFW Builds 759/769

- Both published normally with full assets/CI. PCE Exit/icon issues remained and
  were reported by the owner on 769. `--latest=false` failed to preserve GitHub's
  latest release; it was restored with an explicit REST `make_latest` and
  re-verified.

### 2026-11-17 — SuperFW Build 667 settings fix

- Withdrawn Build 640/Test 6 froze on Interface settings. The cause was a GCC
  16.1.0 ARM Thumb-1 IPA-RA caller/callee register-clobber bug; SuperFW disables
  IPA-RA locally. Save-on-exit/cooperative verified settings and a permanent
  native linked-ARM fixture were added. Build 667 published; only Chis has owner
  hardware observations.

Earlier history is condensed in `PROGRESS.md` and fully archived in
`docs/history/root-journals-2026-11-26/`.

## 8. Immediate next steps

1. Confirm the root-doc restructure is on `dssmm` and synced to the lanes.
2. Finish/integrate the superfw Build 827 workspace change through the normal
   lane → `dssmm` merge, then sync `dssmm` back.
3. Prove a fresh clone can build all four projects and all six board variants
   (DE, OMEGA, BLUE, SuperFW SD/Lite/Chis) from the recorded sources.
4. Implement the owner-requested 19px SuperCard top bar and matching colors
   within the validated memory budget.
5. Continue SuperFW parity work (settings layout/persistence, PCE icon/return,
   plugin exit, NOR/artwork edges, SD/Lite behavior) in `superfw/`.
6. Do not publish Build 822/827, recreate Build 640/Test 6, or resume Atari
   engine work here without an explicit owner request.
