# Atari2600 investigation

The current EZ-FLASH manager PLUG pack has no a26.bin/gba/mb/mbz. SuperFW can
route extension-named external plugins, but that does not supply an Atari core.
Do not alias .a26 to an unrelated emulator or claim support from icon lookup.

Candidate source: https://github.com/MikeDX/2600_gba at
2eb18dd8a5b832c939efe510b4e493ad4cf25437 (read-only upstream clone in the session).
Its headers credit Virtual VCS/X2600, Daniel Boris, Alex Hornby and the x64 team
and state the GNU GPL. Preserve all notices and document component licensing
before adding any distributable source/binary. No upstream prebuilt object or
embedded commercial ROM is to be shipped.

This is an old ARM-toolchain source project. Includes assume a case-insensitive
filesystem, main is C_Entry, cartridge loading references absent embedded ROM
headers, and gba.h defines initialized globals in multiple translation units.
The source already has bank-switch handling and a64000-byte video buffer; it
needs a modern build and bounded runtime cartridge input before integration.

1. Prototype outside the tracked tree: normalize filenames/includes, use a
   current GBA startup/linker and keep safe SuperCard WAITCNT. Replace embedded
   ROM selection with a bounded runtime buffer/size and synthetic4KiB cartridge.
2. Validate CPU/TIA output and controls with generated homebrew, first in stock
   mGBA. Measure emulation speed; source existence is not a playability claim.
3. Add an external SYSTEM/PLUG/a26 plugin interface using existing Pogoshell
   arguments and safe return, with ROM size/bank guards and proper credits.
   Keep this separate from firmware budgets unless bundling is justified.
4. Commit reproducible source/build and real tests only after the prototype
   works; expose compatibility/performance limits. Then provide a hardware test.

This investigation is ongoing. No Atari2600 emulator is bundled or validated by
Build769. PCE return/icon and Start alignment corrections proceed independently.

## Reproducible emulator-only prototype

The pinned source now compiles with current devkitARM and renders an original
4KiB 6502 diagnostic in stock mGBA0.10.1. The exact 160x96 solid-color rectangle
at (35,25) passes the shared PNG pixel reader. No commercial ROM, old binary
object, BIOS, new emulator dependency or firmware asset is used.

Clone the upstream repository outside the tracked tree and checkout the pin
above, then run from the repository root:

```sh
python3 superfw/tools/build-atari2600-prototype.py --source /path/to/2600_gba
bash superfw/tools/test-atari2600-prototype-mgba.sh
```

Generated files stay under ignored `superfw/build/atari2600-prototype/`.
The recipe preserves source notices, normalizes includes/globals, replaces the
embedded-game selector, fixes a 255-entry palette indexed through255, keeps the
bounded half-height renderer and moves large buffers to EWRAM. It is optional;
normal builds and CI do not fetch or bundle the external source.

The prototype now has a ROM-only startup that keeps all96 Pogoshell argument
bytes, clears only owned RAM/VRAM, establishes the bitmap transform and writes
safe default WAITCNT0x4000 before copying cartridge data. The SDK startup did
not establish the suspected fast-wait setting; its whole-EWRAM clear was the
confirmed ABI problem. The experimental loader accepts only2KiB/4KiB input,
validates magic/count/address/size and mirrors2KiB cartridges. It contains no
embedded game in runtime mode.

```sh
python3 superfw/tools/build-atari2600-prototype.py --source /path/to/2600_gba --runtime --out superfw/build/atari2600-runtime
python3 superfw/tools/test-atari2600-input-arm.py
```

Actual linked ARM startup and19 copy/mirror/invalid-input cases pass; the
standalone mGBA diagnostic also passes through this runtime loader. These are
software checks, not physical-card qualification. Safe Exit, controls, sound,
bank selection, speed and full licensing/provenance review remain. Upstream
headers refer to a missing COPYING file; resolve that before distributing an
emulator binary. Keep this prototype out of hardware-test and public assets.

## Owner priorities and alternative review (2026-11-24)

Owner allows either MikeDX/2600_gba or Sterophonick/PocketStella and requests
familiar emulator menu style/structure, L+R entry, D-pad navigation, A select,
B back, and normal Resume/Reset/Exit behavior. Prefer the already rendering
MikeDX engine for the first working plugin; retain PocketStella as a frontend/
engine reference with proper credit.

PocketStella pin1e299b9e6c3197f6ad63a5a06eaf4db1c981bc43 was reviewed from source.
Its src/6502.c is empty, CPU execution is commented out, a26Render/a26Sound are
stubs and the Windows Makefile depends on libheart. build/m6502.s is a partial
object disassembly, not a complete rebuildable core. Its doc/2600_gba.zip is
reference source; do not use bundled prebuilt objects or doc/rom.bin. Public
source: https://github.com/Sterophonick/PocketStella .

Build799 includes an Atari icon only. Owner799 reports Unknown file type for
.a26; no emulator file has been delivered. Generic extension plugin lookup
already supports SYSTEM/PLUG/a26.bin/.gba/.mb/.mbz, so finish and qualify that
external asset rather than pretending an icon or extension registration is an
emulator. Safe Exit, ROM banking and source/license completeness precede the
hardware-test plugin. Keep progressing without requesting owner input.
