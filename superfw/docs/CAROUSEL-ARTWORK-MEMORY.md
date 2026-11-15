# Carousel artwork cartridge-memory defect (2026-11-15)

Owner evidence: Chis Build 553 fixes the first-entry navigation crash; Box
artwork displays in List view but artwork remains absent in carousel views.

List view stores `artwork_cached`, `artwork_valid`, `artwork_rom` and
`preview_info` in GBA RAM. The carousel instead put `cached`, `valid`, the path
string and `artwork_info` inside `t_sdram_state` at 0x08000000. Boolean stores,
`strbcpy` and the transparency byte require byte writes. Cartridge SDRAM cannot
accept these stores correctly. Cache keys/validity therefore fail even when the
same loader and pixels work in List. Repeated cache misses also cause repeated
SD reads. Host mocks and the old mGBA preview used ordinary RAM and missed this.

The fix keeps all three slots' metadata in a separate `carousel_meta` object
in internal GBA RAM, leaving only halfword-written image pixels in cartridge
SDRAM. The existing invalidation and lookup order remain. Normal system icons
replace the owner-rejected Box/Title neighbor placeholders. Unfinished settings
selection styling was set aside; this repair does not resume the broader port.

## Evidence and reproduction

`test-carousel-cache.sh` now extracts the production storage declarations as
well as the renderer/invalidation functions. It tests cache hits/misses,
path/mode changes, motion, both orientations, Animation Off and page reuse.
The optional `test-carousel-cart-bus.py` compiles the same test as ARM7TDMI
Thumb and executes it with Unicorn 2.0.1. It maps test cartridge storage at
0x08000000 and rejects byte or unaligned stores there. Storage, text and blit
calls are mocks; this is a CPU memory-access test, not an SD/card emulator.

With devkitARM on PATH and Python `unicorn` installed:

```sh
git show 3ce7d917:superfw/src/menu.c > /tmp/build553-menu.c
MENU_SOURCE=/tmp/build553-menu.c python3 superfw/scripts/test-carousel-cart-bus.py
python3 superfw/scripts/test-carousel-cart-bus.py
```

Build 553 fails with `unsupported 1-byte cartridge store at 0x80026a0`.
Fixed source passes all cache assertions and the store-width check. This
reproduces a real firmware defect, without claiming physical validation.
`check-carousel-memory.py` additionally inspects the actual firmware ELF on
**every board build**, requiring a separate writable metadata object in GBA
RAM. The first corrected Chis link puts its 816 bytes at 0x03000e74 (IWRAM),
with total IWRAM usage 12,872 / 32,768 bytes. Final addresses may change by build.

No mGBA setup changes are required. Unicorn is an optional diagnostic dependency,
not a new mandatory devcontainer/CI dependency. The normal host regressions and
ELF build gate run without it. mGBA previews still do not exercise cartridge
memory or real FatFS/storage.

## Remaining confirmation

Retest the numbered Chis candidate using the same working Box pack, in both
carousel orientations with Animation On and Off, then Title artwork. A Title
pack that also fails in List may have a separate lookup/format issue; this fix
does not claim to resolve unobserved pack formats. Physical timing and the
owner's full SD contents cannot be reproduced locally. Owner subsequently completed the model switch and authorized resuming styling
after candidate delivery. Build561 is frozen in hardware-test for that retest.
