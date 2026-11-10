# Deferred ezfode review queue

Owner policy: record evidence here during the Original Omega port; do not change ezfode without a separate task.

## ART-HEADER-01 — incomplete marked-BMP validation (review candidate, low impact)

Source reference: ezfode 5f700c9, Launcher_HasTransparentThumbnailMarker. Verified by inspection: it checks DST1, dimensions, negative height, depth and compression, but does not check BM signature, pixel-data offset, DIB length or plane count. The loader assumes a fixed 54-byte header. A buffer with correct checked fields but invalid signature/offset can still pass tagging. Current Omega marker test fixture originally omitted those unchecked fields yet was accepted, corroborating the parser behavior.

Impact remains unverified: this does not demonstrate an out-of-bounds read/write (expected buffer length and pixel count are bounded), or a regression for supported scraper artwork. Review whether malformed files should be rejected before tagging/display. Preserve unmarked legacy-art semantics and use a complete valid fixture plus individual malformed-field tests if tightening validation. No ezfode fix made.

Other inherited Original Omega warnings are tracked in OMEGA_STATUS.md, not asserted to be ezfode defects.

## SAVE-PATH-01 — case-sensitive transaction target policy (verified policy behavior)

At ezfode 5f700c9, FileTxn_IsAllowedTarget("/system/saver/lower.sav") returns 0, while FatFs path lookup is case-insensitive. The function also guards relative paths expanded through f_getcwd, so lower/mixed-case actual directory names may reject legitimate replacements. Standalone extraction/compile reproduced this result. End-to-end occurrence on a DE card is untested. Review ASCII-case-insensitive matching for fixed SYSTEM/SAVER/SETTINGS components while preserving basename validation. Original Omega port adds this matching and a host regression; ezfode unchanged.

## SAVE-PATH-02 — backslash separators allowed in save basename (verified policy behavior)

Same function returns 1 for /SYSTEM/SAVER/..\..\game.gba because it rejects only forward slashes in basename, while FatFs recognizes backslashes as separators. Standalone extraction reproduced acceptance. This violates the intended single-basename restriction. Current caller reachability and any exploitable corruption are unverified; normal FAT directory entries cannot contain a backslash. Review API/journal-validation paths before assigning broader severity. Original Omega port rejects both separator forms and tests it; ezfode unchanged.
