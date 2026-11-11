# Original Omega Merkin Mix port status

## Current state — Test 2 published, cross-platform audit active

Current target is Original OMEGA only. Earlier entries below are historical checkpoints; the current matrix supersedes their pending-feature labels. Hardware remains unavailable. Do not equate DE Rev. A/B or BLUE reports with an Original OMEGA test.

| Area | Current implementation and evidence |
| --- | --- |
| UI/assets | Current Merkin splash/help, DS themes/start/list/carousels, cached transparency-aware motion, clean plugin transitions; lossless splash/NOR asset checks and renderer comparisons |
| Artwork | Filename/mirrored/LFN/header lookup, marked transparency, indexed FAT fast-open with fallback, sorted CUSTOM manifest, extension defaults and full missing-art previews; actual production-path tests |
| Identity/pools | Separate bounded display names and SFN identities; 512 records plus paired directory offsets within64 KiB PSRAM, staged sorting and input validation |
| Settings | Shared boolean/enum matching and migration, editable/cache-aware Clean list, three-state wrap scope, CUSTOM Only rounded corners with cache provenance;1,285/33,600-case differential suites |
| Language/popups | Complete13 x154 runtime language entries (Thai remembered-cheat gap corrected), localized prompts and readable management-popup confirm/back hints |
| NOR art/progress | SD-only OAR1 full source-path hints checked against catalog identity and checksum; themed progress/background/clock/username; original append/newest-delete policy retained |
| Saves/cheats | Staged SD save/settings replacement and remembered/collapsible selections with original runtime engine; real FatFs fault injection and identity/viewport tests |
| Loading/patching |32 MiB target geometry, checked exact I/O/padding/ranges, staged patch intersections, failure-triggered directory rebuild, bounded raw/LZ EWRAM plugin payloads and distinct WRAM/VRAM BIOS wrappers |
| Hardware-specific boundaries | Original Omega cartridge/SRAM/SD/reset/flash protocols retained. DE LEDs/Mode B/FRAM/firmware-update protocols and BLUE64 MiB mapping are not transplanted |

Latest build2,049,144 bytes (48,008 below2 MiB). Static EWRAM heap19,424; IWRAM data-to-user-stack gap7,488. These are reservations, not measured runtime peaks. Full final verifier passed; [Test2](https://github.com/voluptuary/ezfode-dssmm-kernel/releases/tag/v8.3.1-omega-test2) is published and its downloaded asset hash verified. Tests use mocks/sanitizers/real FatFs and linked disassembly, not cartridge hardware.

Known limits: void raw NOR programming APIs and unbounded polling remain, metadata is not power-loss atomic, newest-title-only NOR deletion remains, and optional artwork hints/cheat snapshots are not transactional saves. Malformed-ROM patch-detector indices and other deferred shared findings are scheduled for the subsequent three-branch adversarial audit. Extra custom theme/color slots remain disabled as in the supported donor configuration.

Next: perform the owner-authorized adversarial audit of all three branches. Do not merge platform hardware code or change stable DE release assets as part of this port.

## Historical port checkpoints


## Isolation and provenance

Work only on ezfomega. Do not merge into ezfode, change DE release tags/assets, or use DE candidate filenames for this target. The owner explicitly requires branch isolation. Hardware source baseline: FrankieT19/omega-ds-style-kernel ef30b8ba78147943d95653dfa82f721b52096f67, DS Style v8.0; Merkin Mix source reference: this repository ezfode. Existing authorship/licenses remain in source/history. Wholesale hardware baseline replacement is intentional because this branch previously contained DE code, not an Omega port.

## Inventory / feature matrix

| Area | Status / next action |
| --- | --- |
| Omega cartridge, save-chip, reset and SD bus code | Ezcard_OP.c, reset.s and ff15/ff.c remain reference code. GBApatch.c has software window/trim corrections; NORflash_OP.c has checked input/UI changes, with erase/program routines retained and enumeration bounds/IRQ fixes. ez_define.h gains constants/UI declarations; no DE flash protocol copied |
| Existing DS Style v8 UI / themes / carousels / sound | Reference implementation retained; built with its launcher -Os policy |
| Filename artwork | First batch: custom priority, legacy GBA header lookup, then filename BMP fallback at active artwork root; non-GBA no longer probes arbitrary bytes as GBA header. Bounds/exact reads/closes checked. Mirrored directory/LFN alias resolution is now implemented and host-tested; indexed fast-open now validates offset/SFN/size with ordinary-open fallback |
| Plugin transitions | Merkin guard ported around external-plugin progress strip |
| Marked transparency | Ported DST1 tagging, exact-sentinel blits/scaling, aligned row scan, backing restoration and pool metadata invalidation; host differential passed; hardware pending |
| Opaque/header-art acceleration | Correctness-compatible transparency scan ported; specialized no-scan header path/fast-open remain pending |
| Remembered/collapsible cheats | Ported CHS2, collapse/viewport fixes and localized optional Remember cheats (Off default). Uses target legacy execution engine/table capacity unchanged; host identity/failure/viewport regressions pass. 64-bit file-size bound fixed on port; hardware pending |
| File identity / FatFs fast-open | Implemented browser SFN aliases, paired sorted offsets and O(1) header-art hint lookup; pure FatFs helpers imported while Omega diskio and EWRAM scratch placement are retained; host tests pass |
| Save/settings transactions | Ported staged SD replacement/recovery with Original Omega SRAM sequence and boot settings cache retained; fault-injection suite passes; case-insensitive target policy and both-separator rejection added; physical interruption untested |
| Loader size/I/O/window safeguards | Ported exact-I/O/range/window policy with reference selectors and 32 MiB limit; callers stop on failure; native headers checked without rejecting short plugin files; target build and policy tests pass; hardware pending |
| NOR artwork/progress/error safeguards | SD-input/bounds/catalog-publication safeguards implemented and mocked-flash host-tested; themed NOR writing window/background/username/clock ported and host-tested; visual hardware validation unavailable. Target flash APIs remain void, so physical failures are not detectable by this port yet |
| DE LEDs, hardware settings, firmware update and FRAM behavior | Not transferable by assumption; retain target implementations and assess actual capabilities |
| Extra custom theme/color slots | Reference excludes these for size; remain excluded pending size/RAM evaluation |

## Validation

Use bash scripts/verify-omega-port.sh for the target-specific host checks/build, or make BUILD=build-omega TARGET=omega-port. A 2 MiB post-build guard was ported into the reference Makefile; compiler/linker memory policy otherwise remains reference policy. Existing DE host/layout/audit scripts are not an Omega validation suite; scope them before reuse.

First clean forced build: 2,028,640 bytes, 68,512 below 2 MiB. Layout: EWRAM static end/heap start 0x02039628, heap end 0x02040000; user stack is in IWRAM at 0x03007F00, unlike DE's EWRAM stack policy. This is not a runtime stack bound. Three inherited compiler warnings: two misleading indentation sites and favorite-name suffix truncation; track separately. New filename harness tests actual extracted production functions and passes paths/priority/bounds/read/close failures. Initial host assertion failure was a fixture counter-reset defect, corrected before passing.

No Original Omega hardware tested. Build results do not prove compatibility. Next work: transparency pipeline with differential rendering tests, then target-specific save/loader safeguards and hardware boot/save matrix. Main ezfode remains untouched.

Transparency milestone: production image 2,029,336 bytes (67,816 bytes headroom). Draw output/DMA identical to pinned DE 5f700c9 over the existing differential suite; 766,800 helper cases, zero mismatches. Marker tests cover title/box, malformed/short headers and preserving unmarked black. Low-level hardware files still match reference byte-for-byte. Background restoration call paths were reviewed but still need visual hardware validation. No flash/save tests claimed.

Windows build/clean scripts now also match the Original Omega reference and output ezkernel.bin. Windows execution has not been tested in this Linux workspace. Do not use the inherited DE browser-installer payloads for this target; packaging/installer migration is pending.

Loader milestone: image 2,029,736 bytes (67,416 headroom); added original-target layout guard. Reference EWRAM heap has 27,096 bytes, IWRAM data-to-user-SP gap 7,752 bytes, neither is a runtime allocation/stack peak measurement. Kernel-size guard negative tests pass. Hardware protocol files remain unchanged; ez_define.h only gains the established geometry constants. Three inherited launcher warnings persist.

Save/settings milestone: production image 2,031,816 bytes, 65,336 bytes headroom; EWRAM heap 24,644 bytes and IWRAM static-to-user-SP gap 7,672 bytes. Fault injection passes first-save/full-card/write sweeps/existing-save preservation/settings staging/recovery states. Ambiguous duplicate FAT rename entries remain preserved and require manual recovery; do not claim power-loss atomicity. Host shim removes only EWRAM section annotation from target FatFs, leaving filesystem logic unchanged. Startup cache recovery occurs before cache fill and before streaming reads.

Diagnostic pre-cheat stack snapshot (source 6606239): main 928 bytes; SD_list_MENU 128; Loadfile2PSRAM 80; LoadEMU2PSRAM 120; Save_savefile 624; SavefileWrite implementation 624; Launcher_SaveUnifiedSettings 616; FileTxn_Begin 328, ReadJournal 624, WriteJournal 632, Recover 40; directory resolver 40. These compiler frames exclude nested library/call-chain peaks and do not prove runtime stack safety. Generated stack reports were summarized then removed.

Cheat milestone final build: 2,033,904 bytes; 63,248 headroom; EWRAM heap 21,936 and IWRAM gap 8,048. Runtime assembly unchanged. Remembered snapshots are checksum/identity validated but replacement is not atomic and may lose an old cheat snapshot on write failure; ordinary save transactions have separate safeguards. No physical gameplay, persistence or UI test available.

NOR input milestone: open failure no longer reports success, both callers handle file error, file size/range/catalog slot/name bounds checked, read/seek/close failures abort, final block FF padded, and RAM catalog publication follows complete SD I/O. The Original Omega PPB sequence/erase/program routines remain unchanged; no generic timeout/status protocol copied. Mocked flash tests establish file-I/O ordering, not chip correctness or power-loss safety.

NOR presentation checkpoint: target verifier passes all existing Omega checks plus actual-helper NOR progress bounds/cadence (1..512-block spans) and compressed asset equality. Image 2,043,816 bytes, 53,336 headroom; EWRAM heap 21,936 bytes, IWRAM gap 8,048. Original reset.s exports a VRAM SWI 0x12 wrapper under the legacy name LZ77UnCompWram; reuse avoids pulling libgba Compression.o and duplicate-symbol failure. No reset assembly changed.

Patch/trim checkpoint: PSRAM banked-copy regression first reproduced failure, then passed with splitting. NOR staging guard test reproduced a shared DE helper overflow and now passes clipped/intersected copies across current/next staging windows. Trim tests cover 1/2-byte tails, zero/null guards, exact final reads and all file-operation failures. Full target verifier passes; image 2,044,000 bytes with 53,152 headroom. No flash protocol change. Per-game trim hook retained; tests stub that hook and do not prove all game-specific patches.

Artwork integration correction: initial function-level filename tests did not exercise the separate carousel cache loader. New extracted production-cache tests reproduced the missing non-GBA/NOR fallback; it is now wired for SD caches, root NOR filename art (without unrelated SD source probes), and full-path recents/favorites. Absolute recents derive real parent directories rather than virtual menu labels. .agb native-header fallback preserved. Added 512-byte directory scratch in EWRAM: heap is 21,424 bytes, IWRAM gap 8,048; image 2,044,176 bytes. NOR mirrored install-folder art still needs target-safe metadata path storage; do not claim that feature complete.

Windows packaging now uses explicit TARGET=omega-kernel to match the copied ezkernel.bin input regardless of checkout name. Static review only; no Windows execution available.

Final artwork integration validation: explicit-directory helper also serves Start/favorite previews, where filesystem CWD and displayed currentpath may differ. Forced build plus full target verifier passed at 2,044,200 bytes (52,952 margin). No hardware tests.

NOR scan follow-up: simulated ROM window reproduces slot-64 overrun with stale following header. Guards reject zero/misaligned/oversized catalog spans, recompute offsets, respect kernel base page and restore prior IRQ state even on full-capacity early exit. Full verifier passes; final image 2,044,264 bytes (52,888 headroom), heap 21,424 bytes, IWRAM gap 8,048. NOR physical programming/status and metadata power-loss safety remain unverified. Remaining major areas are indexed file identity/fast-open, verified install-folder/LFN persistence for NOR artwork, and further UI/settings parity; do not assume these are finished.

Indexed identity follow-up: full target verifier passed at2,045,344 bytes (51,808 headroom), EWRAM heap19,936 and IWRAM-to-user-SP gap8,208. Long names use a bounded display prefix and independent alias for filesystem access. Directory offsets remain paired through sorting, and invalid hints fall back. No hardware test. Remaining settings/custom-cache/NOR install-path work is still pending.

Settings/NOR artwork follow-up: current Omega supports shared boolean parsing, editable/cache-aware Clean list, and On, Not SYSTEM wrap scope. SD-only OAR1 artwork sidecars capture resolved install paths for clean/addon NOR installs and are consumed by NOR caches; invalid hints fall back. Focused parsing/scope/cache/record tests pass. Image2,047,432 bytes (49,720 headroom), heap19,424 and IWRAM stack gap7,800 static. Enum/custom-only rounding propagation remains next; hardware remains untested.

Custom/cache/enum follow-up: CUSTOM Only rounded corners now use actual image provenance across cached/scaled/moving/list/popup paths. Sorted CUSTOM manifest lookup and transparency-aware motion are ported, including the DE VRAM installation guard correction. Enum matching passes33,600 cases; manifest queries169,620; view-transition metadata invalidation regression passes. Image2,048,352 bytes,48,800 headroom; heap19,424/stack gap7,720 static. Extension defaults/missing-art and remaining shared loader checks follow before Test2.
