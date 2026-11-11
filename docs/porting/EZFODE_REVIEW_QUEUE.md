# Deferred ezfode review queue

Owner policy: record directly relevant DE findings during the isolated BLUE port; do not change ezfode. The entries below were established during the Omega port against ezfode 5f700c9 and carried forward unchanged in substance. The current DE branch remains at that commit. New BLUE vendor findings are not automatically DE findings.

## ART-HEADER-01 — incomplete marked-BMP validation (review candidate, low impact)

Source reference: ezfode 5f700c9, Launcher_HasTransparentThumbnailMarker. Verified by inspection: it checks DST1, dimensions, negative height, depth and compression, but does not check BM signature, pixel-data offset, DIB length or plane count. The loader assumes a fixed 54-byte header. A buffer with correct checked fields but invalid signature/offset can still pass tagging. Current Omega marker test fixture originally omitted those unchecked fields yet was accepted, corroborating the parser behavior.

Impact remains unverified: this does not demonstrate an out-of-bounds read/write (expected buffer length and pixel count are bounded), or a regression for supported scraper artwork. Review whether malformed files should be rejected before tagging/display. Preserve unmarked legacy-art semantics and use a complete valid fixture plus individual malformed-field tests if tightening validation. No ezfode fix made.

Other inherited Original Omega warnings are tracked in OMEGA_STATUS.md, not asserted to be ezfode defects.

## SAVE-PATH-01 — case-sensitive transaction target policy (verified policy behavior)

At ezfode 5f700c9, FileTxn_IsAllowedTarget("/system/saver/lower.sav") returns 0, while FatFs path lookup is case-insensitive. The function also guards relative paths expanded through f_getcwd, so lower/mixed-case actual directory names may reject legitimate replacements. Standalone extraction/compile reproduced this result. End-to-end occurrence on a DE card is untested. Review ASCII-case-insensitive matching for fixed SYSTEM/SAVER/SETTINGS components while preserving basename validation. Original Omega port adds this matching and a host regression; ezfode unchanged.

## SAVE-PATH-02 — backslash separators allowed in save basename (verified policy behavior)

Same function returns 1 for /SYSTEM/SAVER/..\..\game.gba because it rejects only forward slashes in basename, while FatFs recognizes backslashes as separators. Standalone extraction reproduced acceptance. This violates the intended single-basename restriction. Current caller reachability and any exploitable corruption are unverified; normal FAT directory entries cannot contain a backslash. Review API/journal-validation paths before assigning broader severity. Original Omega port rejects both separator forms and tests it; ezfode unchanged.

## CHEAT-SIZE-01 — exFAT size narrowed before hash limit (verified arithmetic defect)

At ezfode 5f700c9, CheatContentHash assigns 64-bit f_size to u32 before the 1 MiB limit check. A file of 4 GiB + 4 bytes becomes 4 and bypasses the intended bound, hashing only a prefix as though it were the complete input. Original Omega host regression now uses 64-bit FSIZE_t and tests rejection of that size before reading. Fixed only on ezfomega by checking FSIZE_t before narrowing. The GameID mapping reader has a similar size-narrowing site before its 64 KiB cap; reviewed/fixed only on the port. No claim of ordinary game impact or memory overflow; huge/malformed input behavior and hash identity checks need DE follow-up.

Related review: Get_all_Section_val caps produced menu entries, but skipped comment/blank lines do not increase that count. A very large malformed file can still incur substantial parsing time even when hashing is disabled by its bound. Decide a compatibility-preserving total-byte/work budget separately; no such behavior change made during this port.

## PATCH-NOR-01 — NOR patch span can overrun staging block (verified helper defect)

At ezfode 5f700c9, GBApatch.c::Write NOR branch checks only the starting address against windows_offset..+0x20000, then copies size/2 words without clipping the end. A six-byte patch starting at windows_offset+0x1FFFE overwrites four bytes past the 128 KiB staging buffer. A host extraction with an explicit guard region reproduced this failure; the NOR branch was identical to DE before the Omega fix. Actual game-specific caller reachability is not yet established, so do not claim an observed cartridge failure.

Omega now intersects the patch with the current staged window and supports the remaining slice when called for the next window; tests verify both slices and untouched guards. This is a memory safety issue to review on DE separately, including whether every cross-block patch producer re-emits its global patch in the subsequent block. No ezfode change made.

## BUILD-WIN-01 — output filename mismatch in plain-directory build (static review candidate)

At ezfode 5f700c9, build.bat initializes BUILD_OUTPUT_GBA=x.gba, while the Makefile defaults TARGET to the working-directory basename. When no temporary drive is mapped and the project directory is not named x, successful make output may not be copied to the expected kernel .bin. Windows runtime reproduction is unavailable here. Review the no-spaces path case with a differently named checkout. Omega now passes an explicit TARGET=omega-kernel and uses the matching output stem; no ezfode change made.

## BLUE baseline comparison — loader and directory findings

BLUE 109f1a0 has unchecked native reads/ignored loader failures and overlapping directory sprintf appends. These are verified BLUE vendor defects, not newly demonstrated DE defects: current ezfode has checked loading and different directory construction. Do not report them as regressions in ezfode merely because the source shares ancestry. BLUE retains additional unchecked plugin/header/trim paths for subsequent review.

## PLUGIN-HEADER-01 — tiny external-plugin inputs pass through native-header probe

At ezfode 5f700c9, SD_list_MENU unconditionally requires launch_file_size >= 0xB0 and reads GAMECODE at 0xAC after determining is_EMU. Valid supported external-plugin inputs can be smaller than a GBA header (for example a short CHIP-8 program). Source control flow therefore rejects them before LoadEMU2PSRAM despite that loader supporting arbitrary nonempty data. This is a source-verified policy mismatch; hardware occurrence and an installed .ch8 plugin are untested. BLUE will separate native metadata from plugin bounds/close checks. Review all targets with regression cases before a DE fix.

## BOOT-EWRAM-02 — EWRAM plugin reset paths clear the active stack (review in progress)

At ezfode 5f700c9, SetRompageWithHardReset boot modes 2/4/3 clear EWRAM in C, while Makefile puts the user SP at 0x0203F000, inside the cleared range ending 0x0203FBF8. Native mode 0 already uses ResetWithEwramClear, but the plugin branches do not. Need linked-code/machine-execution evidence and bounded plugin-payload checks before fixing DE. BLUE integration hit the related original reset/relocated-stack conflict; no DE changes yet.

## UPDATE-COPY-01 — file copy reports success despite close failure (verified host reproduction)

At ezfode 5f700c9, Copy_file checks f_sync but discards both f_close results and narrows f_size before checking its bound. tests/test-blue-launch-update.py ezfode compiles that production function with a fault model; injected close failure returns success and fails the expected rejection assertion. The BLUE correction checks full size, exact read/write/close outcomes and rejects identical source/destination paths. Review/port the narrow correction during the authorized DE audit, with actual FatFs scenarios where feasible. DE unchanged.

## POGO-PATH-01 — plugin argument uses obsolete fixed directory offset (verified source)

DE 5f700c9 calls make_pogoshell_arguments(plugin + 9, ...), but plugin paths are /SYSTEM/PLUG/name.ext (basename starts at offset 13). This supplies LUG/name.ext instead of the actual plugin basename. Actual compatibility impact depends on whether the plugin uses argv[0]; file payload/address passing is separate. Replace with separator-based basename extraction and test the produced argument record on each affected port/DE in its proper phase.

### PSRAM-INDEX-FAIL-01 — refresh directory after a failed game load

Verified control-flow risk on ezfode 5f700c9: directory records reside in the final page-zero PSRAM region. A sufficiently large partial load overwrites them before a read/close failure. SD_list_MENU returns zero on that failure and its callers refresh only for nonzero results, leaving overwritten records eligible for redraw/navigation. BLUE now returns one after either plugin/native PSRAM load failure, triggering existing directory rebuild paths. Follow up with a DE production-caller regression and retain target banking rules. Severity: medium; physical symptom not hardware-reproduced.
