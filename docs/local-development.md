# Continue DSSMM locally with VS Code, Docker and Ubuntu

This repository includes its development environment in `.devcontainer/`.
Local Docker containers do not consume GitHub Codespaces hours. Clone the Git
repository; do not depend on a downloaded source ZIP or on the old Codespace's
`/tmp`, uncommitted files, absolute paths, chat history or sibling worktrees.

## 1. Prepare the host

For Windows, use Ubuntu under WSL2, VS Code, the WSL extension and the Dev
Containers extension. Enable Docker Desktop's WSL2 integration for that Ubuntu
distribution. Run Git and `code .` from the Ubuntu terminal. Keep the clone in
Ubuntu's Linux filesystem (for example `~/src`), rather than `/mnt/c`, OneDrive
or a Windows shared folder.

For native Ubuntu, use Docker Engine or Docker Desktop and VS Code with Dev
Containers. Confirm Docker is running and `docker version` works before opening
the development container. You do not need to install devkitPro on the host.

Official setup references:
- [VS Code Dev Containers](https://code.visualstudio.com/docs/devcontainers/containers)
- [VS Code with WSL](https://code.visualstudio.com/docs/remote/wsl)
- [Docker Desktop WSL integration](https://docs.docker.com/desktop/features/wsl/)

## 2. Clone and reopen in the container

In the Ubuntu terminal ON YOUR COMPUTER, not this Codespace terminal. Open a
local VS Code window rather than continuing in the Codespace window. Git must
be installed in Ubuntu (`sudo apt-get install git` if needed).


```sh
mkdir -p ~/src
cd ~/src
git clone https://github.com/voluptuary/dssmm.git
cd dssmm
code .
```

In VS Code, press Ctrl+Shift+P, select **Dev Containers: Reopen in Container**,
and allow the first image build to finish. Open a new VS Code terminal afterward;
build commands below run INSIDE the container. If you already created the
container before this update, select **Dev Containers: Rebuild Container**.

The Dockerfile pins `devkitpro/devkitarm:20260610`. It supplies devkitARM/libgba
and GBA rules; the configuration sets DEVKITPRO/DEVKITARM and uses the `vscode`
user. Host validation tools include GCC, Git/GitHub CLI, Python/Unicorn, Node,
Cppcheck, CMake, zlib and OpenSSL headers, lcov for SuperFW host tests, plus
mGBA SDL and Xvfb/xdotool for automated GBA preview sessions. Dev Container
features provide Node24 and normal user utilities. First use requires downloading
container images/features. No package/image cache from Codespaces is required.

### mGBA GBA preview testing

The standard image installs `mgba-sdl`, `xvfb` and `xdotool`. Debian installs the
SDL executable as `/usr/games/mgba`, which may not be on a non-login shell's
`PATH`; check the version with `/usr/games/mgba --version`. The tested distro
package here is 0.10.1. Upstream 0.10.5 has a BIOS-skip fix and can be used
locally if the distro package cannot boot a preview. Run the SuperFW preview
builder with devkitARM on `PATH`, then use Xvfb and dummy audio for headless
sessions:

```sh
bash superfw/scripts/build-browser-preview.sh
cd superfw/build/mgba
SDL_AUDIODRIVER=dummy xvfb-run -a /usr/games/mgba browser-preview.gba
```

`F12` saves an emulator screenshot; xdotool can send navigation keys. The
preview and generated ROM/artwork fixtures are disposable build outputs. The
full SuperFW kernel still needs SuperCard cartridge SDRAM mapping and SD
protocols that mGBA does not emulate, so the preview harness only validates the
shared GBA rendering/artwork modules. If mGBA shows a blank screen, do not count
that as a successful render test; check emulator version, GBA startup/header,
BIOS and skip-BIOS configuration first.

For a repeatable headless smoke check that captures a frame and verifies the
generated Title-art marker, run:

```sh
bash superfw/scripts/test-browser-preview-mgba.sh
```

This requires Xvfb, xdotool, devkitARM and the mGBA executable; set `MGBA_BIN`
if it is installed outside `/usr/games/mgba`.

#### Keeping the standard mGBA setup current

Treat `.devcontainer/Dockerfile`, this section and the root `AGENTS.md` setup
note as one change set whenever mGBA support changes. Record the tested mGBA
version, package source, install dependencies, launch flags, harness changes,
and actual pass/fail result here; add a dated checkpoint to root `HANDOFF.md`
and `PROGRESS.md` so a later session can distinguish an installed tool from a
validated test. If the Docker image or package changes, rebuild the devcontainer
and record that result. Never store a BIOS in the image, repository, release, or
public artifact; BIOS-dependent testing must use a user-provided local file and
must document only its external/local status, not publish it. Keep generated
ROMs, artwork and screenshots disposable and do not call a blank screen a pass.

2026-11-16 harness update: stock Debian `mgba-sdl`0.10.1, SDL `-1`,
`xvfb-run -a`, dummy audio and xdotool remain unchanged; no emulator patches,
Docker/package change or BIOS input is required. SuperFW now adds
`bash superfw/scripts/test-background-mgba.sh`: the actual `__GBA__` DMA renderer
captures12 Light/Dark backgrounds and checks every38400 pixels plus framebuffer
padding guards. All12 passed. PNG decoding is shared with the existing browser
harness in `superfw/tests/png_reader.py`, using only Python's standard library;
no Pillow/new package dependency was added. The browser artwork oracle now
rejects gray-only DS backgrounds instead of counting them as artwork. Optional
`PREVIEW_TIMING=1` shows background/art/total cycle counters; use explicit
`PREVIEW_THEME=6 PREVIEW_FONT=1` for representative colors/font. ROM-fixture
cycles are not physical SuperCard/kernel FPS. Read
[the project performance/validation guide](../superfw/docs/BACKGROUND-PERFORMANCE.md).
Keep all fake ROMs/screenshots/BIOS inputs out of releases.

Current investigation (2026-11-14): this Debian 12 container has `mgba-sdl`
0.10.1 installed at `/usr/games/mgba`; the executable is not on the default
shell `PATH`. The Dockerfile includes the same package, though this container
has not been rebuilt from the Dockerfile change yet. An upstream 0.10.5 AppImage
was also tried outside the repository and showed a blank frame. The packaged
0.10.1 executable successfully displayed a generated synthetic preview ROM under
Xvfb: the 240×160 screenshot showed the horizontal carousel and Title artwork
decoded using a fake GBA header code. This validates only the isolated GBA
preview harness. Box-mode display, keyboard-driven navigation/orientation, SD
path lookup and full SuperFW execution remain unverified; mGBA does not emulate
cartridge SDRAM or SD behavior. The host fixtures separately validate header
codes, checksum and artwork file metadata.

Do not add a GBA BIOS image to the container, repository or releases. BIOS files
supplied for local experiments must stay outside the checkout and remain only
until the session ends, then be removed. Record changes to emulator packages,
versions, launch flags or test harnesses in this guide and the root
HANDOFF/PROGRESS logs so later sessions can reproduce them.

The SuperFW verification dependencies are installed in the container. Rebuild
an existing container after Dockerfile changes. A fresh image build on the owner's Windows/Ubuntu machine is not implied by this repository-side change.

## 3. Orient before changing code

Read root `AGENTS.md` and the CURRENT entries in `HANDOFF.md` and `PROGRESS.md`.
For target work also read that project's AGENTS/HANDOFF/PROGRESS. The current
MiGoomba work and remaining checks are recorded in each project's
`docs/plugins/MIGOOMBA.md`, `migoomba.json`, `migoomba-tests.json` and
`migoomba-build.json`.

```sh
git status --short --branch
make check-isolation
python3 tests/test-project-isolation.py
python3 tests/test-project-merges.py
make
```

`make` alone shows help. Select the correct hardware explicitly:

```sh
make ezfomegade
make verify-ezfomegade
# Or: make ezfomega / make verify-ezfomega
# Or: make ezfblue / make verify-ezfblue
# SuperCard GBA family: make superfw BOARD=sd|lite|chis
# Verify all SuperFW variants and upstream host tests: make verify-superfw
```

In a full checkout, `make build-all` and `make verify-all` select all four.
MiGoomba's generated headers are checked in, so normal kernel builds do NOT
need a MiGoomba checkout, mGBA, commercial games or a network download.

## 4. Work in one local checkout

The simplest local setup is ONE clone and ONE container; switch branches in
its terminal. You do not need to reproduce the Codespace's four worktree paths.
All branches track the same combined layout; develop only in the matching
project directory on its lane (`superfw` is the SuperCard-family lane). Commit or preserve changes before
switching; never reset/force-push to obtain a clean tree.

For DE work, for example:

```sh
git switch ezfomegade
git sparse-checkout set --cone .devcontainer .github docs scripts tests ezfomegade
make verify-ezfomegade
```

Use `ezfomega` or `ezfblue` for the other OMEGA-family cards. For SuperCard work,
switch to `superfw` and include `superfw` in the sparse-checkout set; read its
project AGENTS/HANDOFF/PROGRESS before editing. Sparse checkout hides unrelated
projects but does not delete them from Git. To return to full trunk:

```sh
git sparse-checkout disable
git switch dssmm
```

Merge kernel lanes into trunk with ordinary merge commits; synchronize trunk
back to lanes. See [the full workflow](isolation.md). Keep kernel work/logs
inside its directory; shared root changes/logs belong on trunk. Avoid creating
worktrees outside the container's mounted workspace: host/container absolute
Git paths differ and a sibling worktree may not be mounted in the container.

Configure your Git identity and authentication locally before pushing. Public
HTTPS cloning needs no token. Use your normal Git credentials or `gh auth login`
inside the container for writes. Never put tokens or authentication files in
this repository or copy them into documentation.

## 5. Existing OMEGA-family build guidance

The three OMEGA-family lanes contain a pinned MiGoomba v0.1 replacement and generated
payloads: DE uses `ezode`, Original OMEGA uses `ezo`, BLUE uses `generic` to avoid
unverified DE rumble register writes. Existing .esv paths/64KiB kernel save
handling remain; back up saves. Old saved-state compatibility is not established.

Earlier successful evidence is in committed per-target JSON reports. Fresh local
runs on 2026-11-12 completed the three OMEGA-family verifiers, save/conversion/legacy
replays and focused Cppcheck. Per-target `migoomba-release-validation.json`
records the results for 8.4.0. Rerun checks after relevant changes; interrupted
commands never count as passes.

Optional emulator/save tests need a separate pinned MiGoomba source checkout and
its mGBA harness. Follow the project's MIGOOMBA.md, including zlib headers and
`-D_GNU_SOURCE` for strict-C host harness builds on this Linux setup. The upstream
save tools print FAIL without necessarily returning a failing process code;
check individual results. Do not fetch or commit commercial ROMs/screenshots.

Generated `.elf`, `.gba`, `.bin` and `build*` outputs are ignored and must be
rebuilt locally if unavailable. Do not recreate candidate folders or upload an
extra source ZIP. Kernel source is modified intentionally, so the optional
historical release-input baseline check may report differences; it is not a
merge gate. Earlier DE8.3.2 / OMEGA Test3 / BLUE Test2 releases remain unchanged. The owner
subsequently authorized 8.4.0: DE stable, OMEGA Test4 and BLUE Test3 prereleases;
see current journals and release notes. Do not overwrite earlier releases.

## 6. SuperFW (SuperCard GBA family)

The `superfw/` directory contains the upstream SuperFW snapshot and its independent
license/provenance. Read `superfw/AGENTS.md` and `superfw/docs/UPSTREAM.md`. The
root dispatcher offers `make superfw BOARD=sd|lite|chis` and
`make verify-superfw`; outputs live under `superfw/build/`. Its source has not
been physically tested as part of this repository setup.

For exact timing-counter transcription with the existing preview harness, use
`python3 superfw/scripts/read-preview-timing.py superfw/build/mgba/browser-preview-0.png`
after `PREVIEW_THEME=6 PREVIEW_FONT=1 PREVIEW_VERTICAL=1 PREVIEW_TIMING=1`.
The reader matches DSSMM glyphs with the shared standard-library PNG decoder;
no OCR package, mGBA change or private BIOS is needed. Dark counters require
`--dark`. These fixture cycle counts are not physical firmware FPS.
