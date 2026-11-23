# Emulator plugins

## Build769 hardware follow-up and native PCE candidate

Owner reports GG/SMS operation working at746. At769 PCE games load, but Exit
restarts the emulated game. The report does not identify which backend ran.
The bundled native PCE path lacked the verified return adapter already used
by portable plugins. The candidate applies it only to recognized PCE cores,
scanning only emulator bytes and using the existing validated firmware snapshot
and return stub. MiGoomba and unknown reset patterns remain untouched.

The actual linked loader/FatFs replay checks the adapted core and exact game
append. `DSSMM_PLUGIN_SYSTEMS=pce-native bash scripts/test-plugin-preview-mgba.sh`
boots the native ROM/header layout and checks menu Exit reaches a marker in
stock mGBA. Firmware restoration is covered separately by the production ARM
return tests. These checks do not qualify physical PCE Exit/save timing.

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

Game Gear (`.gg`) aliases the `sms` plugin key, so it uses the same emulator as
`.sms` while keeping its original filename, native GG mode flags and Game Gear
icon. See GAME-GEAR.md for icon credits and two-character save-name coverage.

## Build618 hardware follow-up

The owner reports NES, GB, GBC and NGPC plugins launch on Chis; GG freezes white,
SMS reaches a black screen with an accessible emulator menu, and PCE does not
load. These observations do not qualify gameplay or saves. SMSAdvance and
PCEAdvance in the existing pack both reset WAITCNT to `0x14` during startup,
which is too fast for SuperCard cartridge SDRAM. The loader now recognizes the
complete ARM MOV/LDR/STRH sequence and its WAITCNT literal, replacing just the
MOV value with `0x4000` (default cartridge waits and prefetch). No SD plugin is
modified, and game bytes are never scanned or patched. This applies to recognized
uncompressed BIN/GBA/MB startup code; MBZ timing adaptation remains unsupported.

The owner also reports a crash on emulator Exit. These plugins' cart reset
routines do not provide a verified return to SuperFW, particularly a chain-loaded
copy. This remains a separate compatibility issue; the startup timing correction
does not claim to fix Exit. Power cycling remains the available return method.
Upstream SuperFW already documents a WAITCNT-patched bundled PCEAdvance in
`emu/README.md`; the timing correction follows that hardware requirement.

## 2026-11-21 candidate: exact Exit adaptation and compressed plugins

The above Build618 limits describe the older implementation. The new candidate
also expands validated MBZ streams into separate halfword-only cartridge scratch
before applying the same startup and Exit checks. It reserves SDRAM24..26MiB for
a snapshot of the running firmware and26MiB for a self-contained return stub.
This avoids the existing patch scanner's16..24MiB scratch and the portable
payload's14MiB bound. Boot captures installed or chain-loaded firmware without
writing flash. Exact SMSAdvance2.5/PCEAdvance7.5 reset routines can return through
this snapshot; unrecognized reset code is not rewritten. RAM plugins also receive
a guarded ROM0 SoftReset trampoline. This does not promise arbitrary card-specific
plugins will work, nor does it add emulator-specific save-state support.

Validation uses production ARM boot/copy/reset code, full firmware restoration,
invalid-size and cartridge halfword-store guards. The actual linked optimized
loader/FatFs runs against a FAT16 sector fixture for SMS/GG/PCE, MBZ, BIN and GBA
payloads. Stock mGBA runs real SMS/GG/PCE binaries with synthetic homebrew ROMs and
checks their Exit menu reaches a test marker; production restoration is separately
covered by ARM replay. No commercial game, BIOS or extra plugin binary is added
to the repository. Scripts `test-pogo-boot-arm.py`, `test-pogo-return-arm.py`,
`test-native-plugins-arm.py` and `test-plugin-preview-mgba.sh` retain these checks.
Physical Chis Exit, the owner's PCE load error, saves and SD/Lite operation still
require hardware retesting; emulator success is not that qualification.
