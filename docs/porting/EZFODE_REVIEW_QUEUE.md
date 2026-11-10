# Deferred ezfode review queue

Owner policy: record evidence here during the Original Omega port; do not change ezfode without a separate task.

## ART-HEADER-01 — incomplete marked-BMP validation (review candidate, low impact)

Source reference: ezfode 5f700c9, Launcher_HasTransparentThumbnailMarker. Verified by inspection: it checks DST1, dimensions, negative height, depth and compression, but does not check BM signature, pixel-data offset, DIB length or plane count. The loader assumes a fixed 54-byte header. A buffer with correct checked fields but invalid signature/offset can still pass tagging. Current Omega marker test fixture originally omitted those unchecked fields yet was accepted, corroborating the parser behavior.

Impact remains unverified: this does not demonstrate an out-of-bounds read/write (expected buffer length and pixel count are bounded), or a regression for supported scraper artwork. Review whether malformed files should be rejected before tagging/display. Preserve unmarked legacy-art semantics and use a complete valid fixture plus individual malformed-field tests if tightening validation. No ezfode fix made.

Other inherited Original Omega warnings are tracked in OMEGA_STATUS.md, not asserted to be ezfode defects.
