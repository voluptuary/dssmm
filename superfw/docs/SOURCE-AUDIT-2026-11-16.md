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
processed all40 source C files. The directsave_emu cache-magic warning
is an analyzer limitation: load_cache_metadata writes every byte of the entire
local struct from volatile SRAM before inspecting it. No blanket suppression or
extra runtime initialization was added. Final whole-program analysis also reported two nanoprintf float-to-byte casts
(valid C object-representation access in retained third-party code) and an NDS
optional-driver null dereference. The NDS defect and its early-return file-handle
leaks are now fixed and covered by a synthetic-RAM/fault test. The earlier
first-pass journal saying only the cache warning remained preceded these final
whole-program diagnostics; this paragraph corrects that claim.

The earlier scaler warning disappeared
after the sampling rewrite; duplicate-condition and zero-divisor findings fixed.

Actual ARM artwork/cache bus checks and all10 mGBA background pixels pass.
Shared settings save/reload tests still cover360 combinations per board. Broader
owner-reported physical settings loss remains unreproduced; host tests alone do
not close it. Plugin Exit and physical SMS/GG/PCE timing need further work.

## Native scanner and loader corrections

The scanner now bounds every signature and flash-setup structure read, enforces
its128-operation capacity, and inserts operations without clobbering later IRQ/
RTC groups. Failure propagates instead of caching a partial patch set. Explicit
4096-byte lookbehind and64-byte lookahead windows preserve literal loads and
cross-boundary signatures; setup-table skip state also crosses windows. The CLI
uses exact allocations instead of padding to hide overreads.

Production patch generation verifies file size, seeks and exact reads, pads only
the final word, restores SD mapping and closes the file on failure. Scanner tests
compare complete-ROM output with overlapped windows, including a boundary-spanning
signature/setup table and a maximum-backoff literal. Exact allocation and capacity
sanitizers pass. The actual production reader is exercised with a9MiB+3 synthetic
file, short/error reads and scanner failure; no failed run writes a patch cache.

The NDS loader now owns its handle in one wrapper, accepts a null optional driver
and rejects invalid driver-size shifts. Tests cover synthetic RAM only, not DS
hardware execution. Broader DLDI relocation qualification is separate.

The expanded scanner initially exceeded Lite's IWRAM stack budget by48bytes.
Moving the existing256-byte list/Start artwork filename cache to EWRAM restores
the minimum reserve without moving hot metadata or reducing the stack. Preliminary
Lite link passes with16592bytes user stack; clean all3/full gates remain pending.

After those gates, continue porting the remaining DE menu grouping/navigation,
shared preference behavior and artwork alignment. Physical plugin Exit and the
owner's broader preference-loss report remain open; no hardware input is required
to continue other porting work.

### Third pass: bounded native transfers and persisted-setting diagnostics

Shared loader_layout helpers account for IGM fonts/cheats and validate the ROM
hole before allocation or cartridge writes. Native GBA transfers check size,
exact reads, seeks, close and aligned tails. After destructive cartridge writes,
a failed load presents an internal-ASCII restart-only screen instead of accessing
potentially overwritten font/VFS assets (VFS resides at30MiB). Progress is bounded.
The loader tests exercise a32MiB ROM/internal hole, odd tails and injected I/O
errors; mGBA screen15 covers the asset-independent error screen.

Settings read failures now block overwriting the original file and show a boot
error; embedded NUL input is rejected. Actual production FatFs RAM-disk tests
save/unmount/remount90 UI combinations on both FAT16 and FAT32. This does not
resolve the owner's physical power-cycle persistence report by itself.

The requested GBA Load/Write-NOR selector reuses the Chis backend, default-No
confirmation and capacity recheck. Insufficient occupied capacity offers replacing
all existing games; impossible per-game sizes are rejected before confirmation.
Source size/open/close are checked before removing metadata. Removal uses the
native metadata wipe; reused game blocks are erased by the writer. A subsequent
write failure can leave the confirmed removed games unavailable. SD/Lite retain
only Load. Native NOR writes now guard payload holes/32MiB null-patch cases,
check exact reads/size/close, and preserve odd tails with erased-byte padding.
Mock-flash tests cover cancellation, full/occupied capacity, replacement failure,
source failures,32MiB no-patch writes and odd-byte data. No physical flash writes
were performed during these tests. Full clean candidate gates are still pending.
