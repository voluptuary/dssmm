# Kernel ROM size audit — 2026-11-10

Baseline: 9b23ce8, production devkitARM 16.1.0 flags, 2,092,064 bytes. Kernel limit: 2,097,152 bytes. Scope: linked firmware, asset representation, compiler/linker policy and runtime costs. This is a ROM-size audit, not a new full hardware/functional audit.

## Evidence and implemented change

ELF symbol inventory shows twelve 76,800-byte full-screen theme backgrounds, the 76,800-byte NOR background, two 36,960-byte popup images, 182,737-byte Chinese glyph data, two 87,172-byte code-page tables, 76,472-byte Goomba, 49,184-byte reset table, 36,696-byte startup audio and 9,208-byte compressed splash. Large read-only objects (at least 512 bytes) were hashed from their linked ROM ranges; no identical complete objects were found. Identical-looking images cannot safely be aliased on appearance alone; pointer identity also selects theme top bars.

NOR background can be decoded once directly to VRAM before the progress panel. Its 76,800 raw bytes compress to 9,096 bytes using the existing BIOS LZ77 API. No heap/cache workspace is introduced. Original images/blank/NOR.bmp and RGB555 NOR.h remain canonical assets; generated NOR_lz.h is the linked representation. Generator command: `python3 scripts/generate-nor-background.py`. Timestamp-free generation is deterministic. `--check` needs only Python and verifies BMP-to-RGB555 equality, exact decompressed bytes, output length and VRAM-safe backreferences (distance >= 2). The displayed background and overlay draw order remain identical.

Measured production result: 2,024,344 bytes; saving 67,720 bytes; margin 72,808 bytes. This trades a one-time BIOS decode for the former full-screen copy; no per-frame or flash polling work is added. Physical transition latency has not been measured, so hardware behavior is not declared regression-free solely from host checks.

The optimized ELF contains 1,829,696 bytes of .rodata versus 167,904 bytes of .text, plus relocated code/data. Asset payloads are therefore the primary target. A differential check verified 40 other linked artwork/font/emulator/audio/reset/table payloads are present byte-for-byte in the baseline ROM. Regenerating NOR_lz.h twice produced identical file bytes.

## Other opportunities and regression risks

- Full theme images dominate ROM but support direct/cropped row reads, pointer identity, overlay restoration and custom themes. Whole-image compression would require persistent RAM or repeated decompression. Neither is justified without a separate rendering/buffer design and hardware timing matrix.
- NOR compression recovers most of the 67,704-byte increase from its recent uncompressed representation; retaining the canonical raw header does not consume ROM when it is not included.
- Fonts and OEM936 tables serve configured Chinese and multilingual filenames. Removing them, languages, Goomba or reset tables would remove compatibility. No such change made.
- Splash and audio already have specialized representation/placement. Changing formats requires an independent timing/IRQ review; no lossy assets or playback changes proposed.
- Makefile already enables function/data sections and linker garbage collection with startup roots. Removing unreferenced source files generally saves no ROM. No linker-root/section changes made.
- Newlib floating-capable formatting is linked: `_dtoa_r` is 4,256 bytes and `_svfprintf_r` is 8,926 bytes. These sizes are not guaranteed net savings. Integer-only formatting entry points might remove dependencies, but require complete variadic-format, locale, width/precision and error-path review. Do not blindly replace printf-family APIs or confuse them with FatFs formatting. Deferred rather than changing unrelated behavior.
- Global -Os/LTO/compiler upgrades can change hot loops, patch scanning, memory layout and hardware timing. Prior audit measured smaller -Os images, but this is not equivalent-runtime evidence. Do not apply globally to save bytes without isolated differential and cartridge validation.
- FatFs feature stripping can break exFAT, filenames, or settings/save transactions. Existing supported formats remain enabled.

## Validation and limits

Full scripts/rebuild-and-verify-audit.sh passed: production build, all 34 host scripts (including pixel/decompression regression), browser installer scenarios, JS syntax, whitespace, layout guard and Cppcheck without diagnostics. IWRAM gap remains 1,080 bytes, EWRAM heap 7,112 bytes and user-stack reserve 12,288 bytes. Forty other large linked asset payloads were verified byte-for-byte against baseline; generator output is reproducible. Final source diff changes only the NOR image include and one draw call; no flash operations changed. The new candidate requires a visual/transition check on hardware. Do not replace published release assets automatically. Further feature/code changes must retain the 2 MiB guard.

Candidate: files/candidate/8.3.1-size-audit/ezkernelnew.bin; SHA-256 `8fb4cd6054679680d4b774275f114a885e66b00b07ebc9cbe98837726f1c9bf8`. Stable release remains unchanged.
