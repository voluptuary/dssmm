# Emulator plugins

SuperFW DSSMM can launch extension-named Pogoshell plugins from `SYSTEM/PLUG`.
For example, a `nes.gba` plugin handles `.nes` games, and `sms.mb` handles `.sms`
games. Copy your existing plugin files into that directory; keep their names.
Selection tries `.bin`, `.gba`, `.mb`, then `.mbz`, in that order. Extension
matching is case insensitive on the SD card. SuperFW's external named emulators
and bundled emulators remain fallbacks when no extension-named plugin exists.
A present but unreadable or invalid plugin reports an error rather than silently
launching a different emulator.

The four formats use the Pogoshell argument block and synthetic ROM directory:
`.bin`/`.gba` execute from cartridge SDRAM; `.mb` runs from internal GBA RAM;
`.mbz` uses the GBA BIOS LZ77 decompressor before RAM entry. The selected game
remains in cartridge SDRAM. RAM payloads must fit below the preserved argument
block. Loading checks total space, complete reads, compressed-stream bounds and
aligned cartridge writes. No plugin binaries are redistributed by this change.
The old `SYSTEM/emulators` directory is migrated to `SYSTEM/PLUG` without
replacing existing files; resolve conflicting copies manually if startup reports
one. Native SuperFW filenames such as `pocketnes.gba` still work there.

This supports the portable launch format, not EZ-FLASH card registers or drivers.
A plugin with card-specific SD access, save handling or rumble may require an
adapted build. SuperFW continues its native SRAM save preparation; direct card
access and emulator-specific save/state behavior need hardware testing. The
MiGoomba built into the OMEGA-family kernels is not an external plugin file and
is not transferred by copying SYSTEM. SuperFW's bundled Game Boy emulator
remains its fallback.

## Implementation and validation

The portable ABI was compared with DSSMM/DS Style's existing EZ-FLASH loader;
this implementation retains SuperFW's SuperCard mapping and upstream emulator
support. Credit the Pogoshell/plugin authors, DS Style/DSSMM contributors and
SuperFW author David Guillen Fandos. No EZ-FLASH hardware driver was imported.

`bash scripts/test-pogo-plugins.sh` exercises the actual loader and resolver on
all three compile variants: lookup order, case, bounded argument names, layout,
short final transfers, missing files, short/error reads and close failures.
`python3 scripts/test-pogo-reset-arm.py` optionally uses Unicorn to execute the
production ARM reset stub with modeled BIOS clears/decompression, checking ROM
and RAM entry, preserved arguments, stack and WAITCNT at both RAM speeds. It is
not a physical-card or real-BIOS emulator qualification.

Recovery checks also validated all 20 `.mb`/`.mbz` files in the repository's
existing EZFODE plugin pack against the RAM bounds and complete compressed-stream
validator. This establishes format acceptance, not successful emulator gameplay.
No new hardware candidate or release is implied by these focused checks.
