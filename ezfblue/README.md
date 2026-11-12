# DS Style Merkin Mix — BLUE test port

Merkin Mix brings artwork transparency, name-matched artwork for supported systems beyond GBA, and the DS Style menus to the **EZ-FLASH OMEGA Definitive Edition B (BLUE)**. Menu rendering is highly optimized while retaining transparency compatibility.

**[BLUE Test 3 prerelease](https://github.com/voluptuary/dssmm/releases/tag/v8.4.0-blue-test3) — hardware untested.** This branch targets the newer BLUE cartridge only. The older Definitive Edition Rev. A/B uses [ezfomegade](https://github.com/voluptuary/dssmm/tree/ezfomegade); the original OMEGA uses [ezfomega](https://github.com/voluptuary/dssmm/tree/ezfomega). Their kernel updates are not interchangeable.

The BLUE frontend includes the current Merkin Mix themes, carousel/list views, favorites, recent and last-played games, artwork caches, remembered cheat selections, save/settings protections, and themed NOR progress. It retains BLUE's hardware drivers, firmware, native 64 MiB/5G handling, and NOR allocation format. Compatible plugins use the existing plugin interfaces; their runtime behavior still needs BLUE hardware testing.

This is an experimental port with source, build, host-test and linked ARM/Thumb validation only. Boot, installation, saves, cheats/RTS, plugins and NOR operations remain untested on the cartridge. The larger split image preserves fixed resources, but acceptance by the unpublished BLUE updater is also unverified. See [port status and feature matrix](docs/porting/BLUE_STATUS.md) before testing.

Only install the packed **ezbluekernel.bin** from the BLUE release. The raw development `.gba` is not an installable update.


Merkin Mix builds on FrankieT19's DS Style fork and the EZ-FLASH/Sterophonick kernel lineage. FrankieT19's separately authored v8.0 UI refinements and exploitz86's remembered-cheat work are included in the frontend. BLUE hardware support comes from EZ-FLASH's published BLUE kernel. See [credits and provenance](CREDITS.md) for these and other contributions.

## GB/GBC emulator

This release bundles [MiGoomba](https://github.com/michyprima/migoomba)
for Game Boy and Game Boy Color games. This replacement has source/build/software testing,
but needs hardware testing in DSSMM. Back up `.esv` saves before testing; old
emulator save states are not guaranteed compatible.

## BLUE development improvements

Current development (`8.4.1-blue-dev`) adapts findings from **FrankieT19's BLUE
DS Style port**: a stock-size firmware-free update, complete-image checks,
pending-save protection, read-only NOR browsing, allocation/patch bounds,
separate Mode B preferences and corrected BLUE DDR state loading. It also blocks
unverified persistent NOR save-state patches. Use the official BLUE kernel for
FPGA updates. These changes are not in the Test 3 download linked above and have
not been tested on a cartridge. [Comparison, credit and validation](docs/porting/FRANKIET19_BLUE_COMPARISON.md).
