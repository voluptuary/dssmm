<!-- Current workspace layout -->
Build from the repository root with `make ezfblue` and validate with
`make verify-ezfblue`. All hardware-specific paths below are relative to
`ezfblue/`; universal development-container and CI files are at the Git root.

# DS Style Merkin Mix — BLUE test port

Merkin Mix brings artwork transparency, name-matched artwork for supported systems beyond GBA, and the DS Style menus to the **EZ-FLASH OMEGA Definitive Edition B (BLUE)**. Menu rendering is highly optimized while retaining transparency compatibility.

**[BLUE Test 2 prerelease](https://github.com/voluptuary/dssmm/releases/tag/v8.3.1-blue-test2) — hardware untested.** This branch targets the newer BLUE cartridge only. The older Definitive Edition Rev. A/B uses [ezfomegade](https://github.com/voluptuary/dssmm/tree/ezfomegade); the original OMEGA uses [ezfomega](https://github.com/voluptuary/dssmm/tree/ezfomega). Their kernel updates are not interchangeable.

The BLUE frontend includes the current Merkin Mix themes, carousel/list views, favorites, recent and last-played games, artwork caches, remembered cheat selections, save/settings protections, and themed NOR progress. It retains BLUE's hardware drivers, firmware, native 64 MiB/5G handling, and NOR allocation format. Compatible plugins use the existing plugin interfaces; their runtime behavior still needs BLUE hardware testing.

This is an experimental port with source, build, host-test and linked ARM/Thumb validation only. Boot, installation, saves, cheats/RTS, plugins and NOR operations remain untested on the cartridge. The larger split image preserves fixed resources, but acceptance by the unpublished BLUE updater is also unverified. See [port status and feature matrix](docs/porting/BLUE_STATUS.md) before testing.

Build with devkitPro/devkitARM and libgba: `make`. Run `bash scripts/verify-blue-port.sh` for the full software checks (host C compiler, Python 3, `python3-unicorn` and `python3-distutils` required). Active source is under `targets/blue/source`; shared artwork is under `images`. The unused DE donor tree has been removed; only BLUE firmware is present. The raw `.gba` must be packed with its ELF and the vendor resources; it is **not a standalone update**. [Build and packaging instructions](targets/blue/README.md).

Merkin Mix builds on FrankieT19's DS Style fork and the EZ-FLASH/Sterophonick kernel lineage. FrankieT19's separately authored v8.0 UI refinements and exploitz86's remembered-cheat work are included in the frontend. BLUE hardware support comes from EZ-FLASH's published BLUE kernel. See [credits and provenance](CREDITS.md) for these and other contributions.

## Repository isolation

All branches track the complete combined tree. Develop on `ezfblue` inside `ezfblue/`; optional worktree-local sparse checkout hides the other projects without removing them from Git. Run `make verify-ezfblue` from the Git root. Merge this hardware branch normally into `dssmm`, then merge `dssmm` back for integration sync. Keep platform progress in this directory's journals. Shared root changes belong on `dssmm`; see [the shared-tree workflow](../docs/isolation.md). Published release tags are preserved.
