# SuperFW source audit after Test5

Released baseline: `superfw-test5`, source `a4cafcaf`, Build624. Audit changes
remain separate from those published images. Review covers source correctness,
cartridge-bus constraints, memory budgets, rendering cost and maintainability.
This is a continuing audit, not a claim that all hardware paths are qualified.

## Corrected in the first pass

| Finding | Correction and evidence |
| --- | --- |
| Parent-path scratch had255bytes for a255-character path plus NUL | Use bounded256-byte storage; sanitizer test covers exact maximum, oversize and empty paths. |
| Recent reader could parse tails of overlong lines and replace history after a failed read | Bounded streaming line parser discards whole invalid lines, preserves full-length SD/NOR paths and CRLF, and prevents writes after a failed load. Real file/fault fixtures pass. |
| Clipped scaler divides for each sampled column/row | Exact remainder stepping;132 fractional/cropped/transparent cases compare every screen pixel with the division oracle. Synthetic art cycles decrease42%, total20%; not physical FPS. |
| QR Help pictures are orphaned after opaque-panel replacement | Retired the unused API/streams/pixels, regenerated DBR2 with ten active backgrounds. Blob shrinks25332bytes; all source-pixel/guard tests pass. |
| SD benchmark can divide by zero | Reject zero elapsed time alongside existing errors. |
| Duplicate flash precondition and old split-settings save alias | Removed duplicate check and routed clock saves directly to unified save_settings. |
| Custom strcmp/strncmp depend on signed-char defaults | Compare unsigned bytes as required by C; all255×255 nonzero byte pairs tested. |

The release's576-byte settings text cache remains in EWRAM and hot drawing code
in IWRAM. No new image cache or hardware driver is added; full firmware gates
must retain the16KiB user-stack reserve and board-specific flash limits.

## Static analysis and limits

Cppcheck2.10 warning/performance/portability with inconclusive checks and `--force`
processed all40 source C files. The remaining directsave_emu cache-magic warning
is an analyzer limitation: load_cache_metadata writes every byte of the entire
local struct from volatile SRAM before inspecting it. No blanket suppression or
extra runtime initialization was added. The earlier scaler warning disappeared
after the sampling rewrite; duplicate-condition and zero-divisor findings fixed.

Actual ARM artwork/cache bus checks and all10 mGBA background pixels pass.
Shared settings save/reload tests still cover360 combinations per board. Broader
owner-reported physical settings loss remains unreproduced; host tests alone do
not close it. Plugin Exit and physical SMS/GG/PCE timing need further work.

## Next inspection

The native patch scanner's CLI explicitly allocates extra padding to mask
out-of-range reads. Inspect all signature/flash-structure bounds, patch-table
capacity, chunk overlap and production read/error handling together before
changing this path. Preserve valid matches across chunk boundaries and never
cache a partial patch set after failure. Continue menu parity after the audit.
