# Cross-platform adversarial audit — November 2026

Software audit and delivery completed on 2026-11-11. [Downloaded release verification](release-verification-2026-11.json) records exact commits, asset sizes/hashes, prerelease flags and unchanged stable latest. Further operational details are in HANDOFF.md and PROGRESS.md. This is not hardware qualification or a proof of all possible behavior.

## Scope and architecture

This is a GBA flashcart kernel, not a new emulator core. Audited active C firmware, assembly boot boundaries, cartridge RAM/staging/index lifetimes, FatFs identity and save replacement, patch generators/writers, NOR loading/catalog paths, artwork parsing/rendering/caches, settings/cheats, packaging, assets and target resource layouts. Embedded emulators, FPGA firmware and third-party plugin binaries do not supply their CPU/video/audio implementations here; their emulation correctness cannot be established from this source audit.

Baselines: DE5f700c9, Omega20a1d94 and BLUEdb3db27. Work used isolated worktrees/commits with no cross-platform merge or borrowed flash/save/reset protocol. Older source-only deferred queues are superseded by this report; historical entries remain as evidence.

## Verified findings and corrections

| Finding | Severity | Affected branches and correction | Evidence |
| --- | --- | --- | --- |
| PATCH-NES-02: unchecked branch/probe reads and stale detector state | Medium | All three: explicit available spans, complete signatures, pointer/alignment/window checks and retained probe state | Eight baseline failures; ten production scenarios, including5,000 poisoned-tail iterations, pass ASan/assertions |
| PATCH-NOR-01: staged patch copy crosses128 KiB buffer | Medium | DE: intersect each copy with the current window; ports already clipped | Six-byte end-of-window guard corruption reproduced;28 intersection/canary combinations pass |
| Native patch destination beyond cartridge RAM | Low, defense in depth | All three: whole-range preflight using native32/64 MiB capacity before any selector/write | Bank boundaries, final legal halfword, overrun/null/huge spans; producer reachability not proven |
| BOOT-EWRAM-02: plugin clear overwrites C stack; decoder executes replaced ROM | Medium | DE: IWRAM no-return handoff, safe SP, IRQ masked before mapping, direct WRAM BIOS decoder, bounded raw/LZ input | Linked baseline reproduces modes2/3 stack overwrite and mode4 ROM fetch; nine linked scenarios now pass, including maximal raw payload and fallback mode; BIOS/cartridge mocked |
| UPDATE-COPY-01: close failures reported as successful copy | Medium | DE: exact reads/writes, explicit sync, both checked closes, full-width size guard, failed-output cleanup and artwork-cache invalidation | Actual production failure fixtures; ports already carried checked copy paths |
| PLUGIN-HEADER/POGO-PATH/PSRAM-INDEX-FAIL | Low compatibility/recovery | DE: native-only GBA header requirement, correct plugin basename, failed loads request directory rebuild | Actual metadata fixture covers tiny plugin/native/empty/huge/short/read/close; caller rebuild contract checked |
| SAVE-PATH-01/02 and OEM-PATH-03 | Medium port compatibility; Low DE policy | All three: case-insensitive fixed prefixes and shared CP936-aware separators for transactions, backups and artwork | Actual FatFs accepts0x815C; baseline policies reject valid names; transactional readback, encoded backup/directory/basename and ASCII traversal tests pass |
| CHEAT-SIZE-01: narrowing before size limit | Low malformed-input identity | DE: full-width hash limit and GameID-prefix clamp; ports already corrected |4 GiB+4 baseline hash-validation failure reproduced, corrected persistence/row tests pass |
| ART-HEADER-01: malformed marked BMP accepted | Low | All three: BM,54-byte offset,40-byte DIB and planes checked in marked parser only | Both widths,26 malformed field bytes, marker/length/orientation/sentinel/canary tests; opaque/header-art paths unchanged |
| SAVE-LOAD-04: size wraps before prefix clamp; close failure ignored | Low malformed input/recovery | All three: clamp full size before narrowing and stop before SRAM writes on close error | Zero/small/both-bank/128 KiB/oversized/4 GiB+ inputs; read/short/close failures preserve SRAM |
| BUILD-WIN-01: output stem differs from Makefile target | Low | DE: explicit matching target and copy stem for plain/mapped paths | Static contract checked; Windows runtime unavailable |
| Local printf argument types | Low portability | All three: unsigned cheat count and explicit unsigned RTC year | ARM compiler builds and focused configured static checks; valid displayed values unchanged |

No new Critical or High execution/memory defect was established in this audit. The known NOR interruption risks below remain unresolved.

## Findings, changes and remaining work by branch

### ezfode — Definitive Edition Rev. A/B

Applied all shared corrections above plus DE-only staged patch clipping, safe plugin boot, checked copy completion, tiny-plugin/basename/index recovery, cheat-size and Windows fixes. Moved the unchanged832-byte NES patch payload from IWRAM to const ROM; all three payload copies match SHA256 cb7ba0c02daab7dd18701431e8e4fe4c86d8a89e2cb8ac4892c340c24ab4f9c7. Hot renderer maps stayed in IWRAM. Filename scanners are shared rather than independently maintained.

Unreleased audit candidate:2,025,056 bytes;72,096 below2 MiB. IWRAM static-to-IRQ gap1,688 bytes, EWRAM heap7,112 and user-stack reserve12,288. Candidate SHA2568414503f41b218264f0982cef448ca9a160b9fafe69e815854aa4e387aa43857. Stable v8.3.1 tag/asset/latest remain unchanged; earlier Rev. A/B hardware reports do not validate this new candidate.

Remaining: F-022 raw flash polling has no chip-qualified timeout; F-032 NOR catalog replacement remains single-copy/interruption-sensitive; F-036/FatFs rename interruption can require manual SD recovery. New boot/save/NOR behavior still needs physical Rev. A/B regression testing. Windows batch changes need Windows testing. Static stack margins are not measured runtime peaks.

### ezfomega — Original OMEGA

Completed supported software backports before this audit: indexed long-name identities/header fast-open, optimized transparency-compatible menus/motion, CUSTOM Only corners, scoped wrapping/Clean list, extension/missing artwork, SD NOR artwork hints, updated splash/help/localization and popup hints, checked copies/load recovery and bounded plugin payloads with corrected WRAM decoder. These preserve Original OMEGA hardware protocols and32 MiB loading limits.

Audit added shared NES/state/native-capacity, CP936 save/backup/artwork, marked-BMP and save-load corrections, shared path handling and832-byte const-ROM optimization. No DE/BLUE hardware features were imported. Published [Omega Test3](https://github.com/voluptuary/dssmm/releases/tag/v8.3.1-omega-test3):2,049,416 bytes;47,736 below2 MiB; heap19,424; IWRAM static-to-user-SP gap8,312. SHA256c1d2d2965dea571b9825342f5c6fe4747336664c29f036f9983044e28a170a52.

Remaining: no Original OMEGA hardware reports at all. Installation, native/embedded/plugin boot, cheats/save states, saves, timing/audio and NOR require hardware checks. Original NOR polling/program-success reporting and single-copy metadata retain vendor limitations; deletion remains newest-title-only. SD artwork hints/cheat snapshots are best effort. Extra custom color/theme slots and DE/BLUE-specific hardware features are excluded.

### ezfblue — newer OMEGA Definitive Edition B / BLUE

DSSMM frontend, name/transparency artwork, optimized menus, themes, settings, remembered cheats and themed NOR progress are integrated against BLUE-specific native64 MiB/5G loading, NOR allocation/catalog and firmware. Audit added the shared NES/state/native-capacity, CP936 paths, marked-BMP, save-load and format corrections. Its NES payload was already const in ROM; do not count832 bytes as newly freed here.

Published packed [BLUE Test2](https://github.com/voluptuary/dssmm/releases/tag/v8.3.1-blue-test2):3,982,696 bytes; heap15,760; user-stack reserve12,288; IWRAM overlay-to-IRQ gap1,656. SHA256ce7b74449b208f466636aa2c090a4621b7f48a08626ccbcc986e256401973710. ELF-aware packaging preserves vendor resources at1 MiB, firmware0x08195000, asset start0x08207000 and metadata boundary0x08780000. Raw development GBA is not an installable update.

Remaining: no BLUE hardware reports. The unpublished updater's acceptance of the larger split image is unknown even though source/layout/resource checks pass. Native64 MiB/5G, SD/FPGA offload, saves, cheat engine and NOR readback are software-only evidence. Vendor polling has no timeout; catalog/allocation metadata are single-copy. Generic plugin combinations remain32 MiB; extra custom theme/color slots excluded.

## Validation and static-analysis triage

- DE: production build;40-script aggregate host suite plus added save-load suite (41 registered checks collectively), browser installer and JavaScript syntax; linked handoff including maximum payload/fallback; renderer and cache/lookup comparisons.
- Omega: complete scripts/verify-omega-port.sh passes, including real FatFs save/identity/lookup, language, NOR/patch/trim/loading/progress, cache/motion/artwork and plugin policy/wrapper tests. An older extraction fixture was updated to consume the real capacity macro; no product check was disabled.
- BLUE: complete scripts/verify-blue-port.sh passes: target-specific packaging/load/NOR/metadata/IRQ/folder/update/boot tests,24 shared suites against actual targets/blue/source, new parser/path/save checks, ELF-aware pack and RAM guards. Retained root donor C is not BLUE validation.
- Shared tests include20,000 malformed plugin-policy cases,5,000 poisoned-tail NES cases,766,800 renderer/helper comparisons with zero mismatches, exact I/O and real-FatFs transaction faults. These are not physical SD latency, throughput, gameplay or power-loss results.
- scripts/check-kernel-cppcheck.sh analyzes all active C units using actual ARM GCC preprocessing and32-bit types. Only bulk immutable numeric array data become opaque extern declarations with identical types/dimensions/line counts; code and mutable/volatile arrays remain. Input-fidelity tests verify this; payload bytes/decompression/layout are checked separately. Raw incomplete/high-memory attempts are not counted as passes.
- Final Cppcheck covers DE15/Omega17/BLUE16 units; focused checks repeat after final format changes. Constant Start-style conjunctions are intentional compile-time alternatives. FatFs partial-object warnings were traced to initialized fields on selected FAT/exFAT paths; null-string sentinel terminates before an extra index. Installed libgba signed DMA bit31 macros and newlib's unused __libc_cond_init missing return remain dependency diagnostics. No installed headers or safety checks were altered to hide warnings.

## Shared residual limits and next validation

1. Obtain platform hardware and test each model separately; do not infer port success from DE reports. Start with updater acceptance on BLUE, then native/plugin boot, saves/settings across power cycles, NOR write/delete and failure recovery, cheats/save states, artwork/audio/frame timing and maximum-size loads.
2. F-022 requires supported chip timing/status evidence; F-032 requires a proven spare-sector/compatible metadata scheme. Do not choose arbitrary polling deadlines or copy another platform's commands. SD transaction safeguards do not make FAT renames power-loss atomic.
3. Measure stack/heap high-water under worst-case workloads. Current linker/static guards prevent reserved-pool overlap but do not prove a runtime maximum. No new dynamic allocator or larger RAM pool was introduced.
4. NES signatures/patch probes use synthetic valid fixtures. Uncommon formats whose probe arrives after its output block remain unqualified; the bounded implementation does not read unseen future windows or promise retroactive patching.
5. Cheat menu storage is bounded, but huge comment/blank streams still take file-length-proportional parsing time. No speculative total-file cap was imposed that could reject existing compatible databases; a cancellation/work-budget design remains a Low responsiveness follow-up.
6. External SDK/newlib diagnostics should be addressed with supported dependency updates and target validation, not ad hoc edits to installed headers. CP936 scanning must be revisited if FF_CODE_PAGE changes.

No further verified fix with an appropriate regression check and acceptable compatibility risk remains selected in this software-audit scope. Hardware-dependent risks and the explicit coverage limits above are not declared solved. No new stable release was authorized for either port.
