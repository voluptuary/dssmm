# Linked-firmware settings fixture

The fixture sources, build generator and test runner are tracked in this project;
a fresh standard devcontainer can regenerate them without private files or BIOS:

```sh
bash superfw/scripts/build.sh chis
bash superfw/scripts/test-native-settings-mgba.sh
```

Required tools are devkitARM GCC/binutils/GDB, Python/Unicorn, stock mGBA SDL,
Xvfb and xdotool. The standard devcontainer already supplies them. Tested with
GCC16.1.0 and mGBA0.10.1, `-1`, Xvfb and dummy SDL audio; no emulator patches or
BIOS are used. Keep the native compiler workaround in Makefile.

The generator reads the linked ELF, queries struct offsets from its debug data,
and embeds its actual loadable segments in a test ROM. The ROM loads those
segments into native GBA RAM and executes the real menu, renderer, setting
formatter/parser and FatFs code. Only the physical SD read/write entrypoints are
redirected. Native fonts/backgrounds remain at the production ROM resource
address. No OMEGA-family driver or storage code is involved.

The virtual disk is a64KiB sparse FAT12 volume; its low32KiB persists in emulated
SRAM, enough for these settings files. The fixture rejects nonzero writes beyond
that backing store. It is deliberately not a replacement SuperCard driver.

Boot1 exercises real menu keys for Theme, Artwork, Accent and Browser view, exits
to save, reloads the native program state and checks restored values. Boot2 is a
new emulator process reading the same `.sav`; it checks persistence and moves the
selection to the last Interface row. Green corner markers indicate completed
assertions, not hardware qualification. The script checks the panel's lower
bevel against the original pixels. BIOS and commercial ROMs are unnecessary.

Generated ROM/ELF, virtual SD save, fixture metadata and screenshots live under
ignored `superfw/build/native-settings/`. `boot-1.png` and `boot-2.png` are reviewable
screenshots. Re-running the automated test resets only its own generated save.
For interactive checks, launch `native-settings.gba` manually with stock mGBA;
keys use the standard mGBA mapping. Never include fixture ROMs/saves in releases.

Keep this fixture and the smaller preview harness complementary: this one checks
actual optimized firmware behavior; `test-browser-preview-mgba.sh` provides
focused menu/artwork/geometry scenes. Use mGBA screenshots and frame-bound checks
for UI milestones so clipping and stale carousel backgrounds cannot be accepted
from static source review alone. Physical SD/NOR, saves/gameplay and real card
power cycles still require hardware testing.


## Native visual matrix

Run `bash superfw/scripts/test-native-menu-visuals-mgba.sh` after the linked ELF
is available. The verifier runs it after the two-boot persistence check. Scenes
cover Help, System, GBA launch, Chis NOR launch, firmware update, erase confirmation,
loading options, patching options and a bottom-row Orange accent with descenders.
NOR scenes are omitted on unsupported boards. Generated `visual-1.png` through
`visual-9.png` are reviewable artifacts, not tracked assets.

Each scene seeds magenta pixels in the back page before calling the actual native
renderer. Assertions require the old artwork to be completely cleared and compare
the frame's edges/bevel to the original source pixels. Seeding only the back page
is essential: overwriting the displayed front page would create a fixture artifact.
The matrix found popup help descenders crossing the lower bevel and the Orange
accent's last-row descender touching it; DS baselines now keep both inside.

Additional fields are read from `ptype /o` DWARF layouts, including optimized
objects that GDB cannot materialize as live addresses; GDB Python is not required.
Visual scenes are read-only. Interactive Interface testing accepts directional
keys and B only, preventing fixture navigation into destructive tools. No physical
flash commands are issued by the automated tests. Never run fixture ROMs on cards.
