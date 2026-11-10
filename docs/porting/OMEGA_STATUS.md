# Original Omega Merkin Mix port status

## Isolation and provenance

Work only on ezfomega. Do not merge into ezfode, change DE release tags/assets, or use DE candidate filenames for this target. The owner explicitly requires branch isolation. Hardware source baseline: FrankieT19/omega-ds-style-kernel ef30b8ba78147943d95653dfa82f721b52096f67, DS Style v8.0; Merkin Mix source reference: this repository ezfode. Existing authorship/licenses remain in source/history. Wholesale hardware baseline replacement is intentional because this branch previously contained DE code, not an Omega port.

## Inventory / feature matrix

| Area | Status / next action |
| --- | --- |
| Omega cartridge, save-chip, reset and SD bus code | Reference preserved byte-for-byte in Ezcard_OP.c, GBApatch.c, NORflash_OP.c, reset.s, ff15/ff.c, ez_define.h; no DE protocols copied |
| Existing DS Style v8 UI / themes / carousels / sound | Reference implementation retained; built with its launcher -Os policy |
| Filename artwork | First batch: custom priority, legacy GBA header lookup, then filename BMP fallback at active artwork root; non-GBA no longer probes arbitrary bytes as GBA header. Bounds/exact reads/closes checked. Mirrored directory/LFN alias resolution is now implemented and host-tested; indexed fast-open remains pending |
| Plugin transitions | Merkin guard ported around external-plugin progress strip |
| Marked transparency | Ported DST1 tagging, exact-sentinel blits/scaling, aligned row scan, backing restoration and pool metadata invalidation; host differential passed; hardware pending |
| Opaque/header-art acceleration | Correctness-compatible transparency scan ported; specialized no-scan header path/fast-open remain pending |
| Remembered/collapsible cheats | Ported CHS2, collapse/viewport fixes and localized optional Remember cheats (Off default). Uses target legacy execution engine/table capacity unchanged; host identity/failure/viewport regressions pass. 64-bit file-size bound fixed on port; hardware pending |
| File identity / FatFs fast-open | Pending; do not replace Omega filesystem/storage paths wholesale |
| Save/settings transactions | Ported staged SD replacement/recovery with Original Omega SRAM sequence and boot settings cache retained; fault-injection suite passes; case-insensitive target policy and both-separator rejection added; physical interruption untested |
| Loader size/I/O/window safeguards | Ported exact-I/O/range/window policy with reference selectors and 32 MiB limit; callers stop on failure; native headers checked without rejecting short plugin files; target build and policy tests pass; hardware pending |
| NOR artwork/progress/error safeguards | SD-input/bounds/catalog-publication safeguards implemented and mocked-flash host-tested; themed progress/artwork pending. Target flash APIs remain void, so physical failures are not detectable by this port yet |
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
