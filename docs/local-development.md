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
Cppcheck, CMake and zlib headers. Dev Container features provide Node24 and the
normal user utilities. First use requires downloading container images/features.
No package/image cache from Codespaces is required.

The dependency addition was syntax/configuration checked during urgent handoff;
a fresh local Docker image build still needs verification. Do not report it as
already tested on the owner's Windows/Ubuntu machine.

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
```

In a full checkout, `make build-all` and `make verify-all` select all three.
MiGoomba's generated headers are checked in, so normal kernel builds do NOT
need a MiGoomba checkout, mGBA, commercial games or a network download.

## 4. Work in one local checkout

The simplest local setup is ONE clone and ONE container; switch branches in
its terminal. You do not need to reproduce the Codespace's four worktree paths.
All branches track the same combined layout; develop only in the matching
kernel directory on its hardware lane. Commit or preserve changes before
switching; never reset/force-push to obtain a clean tree.

For DE work, for example:

```sh
git switch ezfomegade
git sparse-checkout set --cone .devcontainer .github docs scripts tests ezfomegade
make verify-ezfomegade
```

Use `ezfomega` or `ezfblue` for the other cards. Sparse checkout hides unrelated
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

## 5. Resume the MiGoomba task

The three hardware lanes contain a pinned MiGoomba v0.1 replacement and generated
payloads: DE uses `ezode`, Original OMEGA uses `ezo`, BLUE uses `generic` to avoid
unverified DE rumble register writes. Existing .esv paths/64KiB kernel save
handling remain; back up saves. Old saved-state compatibility is not established.

Earlier successful evidence is in the committed per-target JSON reports. The
session limit discarded live process handles and `/tmp` logs; the last full
verifier, optional replay and Cppcheck runs were NOT recorded as completed.
Run the full verifier for each platform locally and record actual exit results.
Do not infer success from an interrupted command or a previous build report.

Optional emulator/save tests need a separate pinned MiGoomba source checkout and
its mGBA harness. Follow the project's MIGOOMBA.md, including zlib headers and
`-D_GNU_SOURCE` for strict-C host harness builds on this Linux setup. The upstream
save tools print FAIL without necessarily returning a failing process code;
check individual results. Do not fetch or commit commercial ROMs/screenshots.

Generated `.elf`, `.gba`, `.bin` and `build*` outputs are ignored and must be
rebuilt locally if unavailable. Do not recreate candidate folders or upload an
extra source ZIP. Kernel source is modified intentionally, so the optional
historical release-input baseline check may report differences; it is not a
merge gate. Existing published DE8.3.2 / OMEGA Test3 / BLUE Test2 releases remain
unchanged and must not be overwritten as part of environment migration.
