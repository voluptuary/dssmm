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
