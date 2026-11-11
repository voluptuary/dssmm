# BLUE port status

Current audit delivery: software audit complete; hardware-untested Test2 is prepared with shared bounds/path/save/artwork fixes. See [audit findings and limits](../audits/cross-platform-2026-11.md) and [Test2 notes](../releases/blue-test2.md). Publication verification is in HANDOFF.md. Final packed kernel3,982,696 bytes; heap15,760; IRQ gap1,656; user reserve12,288 (static only). Its NES blob was already ROM-resident before this audit. Older sizes/next steps below are historical.


## Current state — DS Style integrated, hardware untested

Active branch `ezfblue`. The frontend now uses Merkin Mix `ezfode` 5f700c9; target-specific hardware is based on EZ-FLASH BLUE 109f1a050d9c35008379ce61ae40d05dc528c614. Older baseline-only entries below are historical, not the current feature status. No physical BLUE results exist. DE and original OMEGA branches were not changed during this integration.

| Area | Implemented and software evidence | Remaining limits |
| --- | --- | --- |
| DS menus/themes/navigation | Current frontend, assets, start/auto themes, carousel/list, favorites/recent/last-played; 24 shared software/UI suites | Frame timing, audio and appearance need hardware |
| Artwork | Name/header/mirrored lookup, marked transparency, borders, scaled rendering, caches; differential renderer and lookup tests | Malformed artwork follow-up remains in audit queue |
| Native game loading | Exact I/O, FF padding, full-width size validation, all eight 8 MiB banks; native game page 0x400 and vendor 64 MiB/5G handling retained | Physical 32/64 MiB games and FPGA offload untested |
| Plugins/embedded GB | Current GB payload and plugin loader; combined plugin ROM bounded to 32 MiB, EWRAM payload/arguments bounded; LZ structure/backreferences checked | Plugin-specific support comes from each plugin; physical execution untested |
| Boot/reset | BLUE native resets retained, IWRAM no-return handoff switches off EWRAM stack before clearing; linked modes 0–6 exercised in Unicorn with BIOS mocks | BIOS mocks do not qualify real hardware |
| Saves/settings/cheats | Transactional SD saves/settings, bounded paths and cheat snapshots; real FatFs fault-injection tests; full-width cheat size bounds | BLUE save/reset/SRAM protocol and vendor runtime cheat engine retained; no claim of DE engine extensions |
| NOR library | 132-byte vendor catalog, 960-page allocation map, free-run install, selected-title delete, game-only format; exact reads, padding and delayed publication; allocation/scan tests | Raw vendor toggle polling still has no timeout; catalog and map are single-copy, not power-loss atomic |
| NOR errors/art/progress | Erase/program/metadata readback, reserve/publish/release ordering; SD artwork sidecars validated by catalog identity; themed NOR background/time/name | Readback and bus behavior mocked; sidecars can fall back after SD failure; no physical NOR results |
| Directory/RAM pools | 256 folders at PSRAM 0x08FE0000, files/offsets at 0x08FF0000; DMA16 staging, no byte writes or PSRAM-to-PSRAM DMA; rebuild after load failure | Static bounds are not runtime stack/heap measurements |
| Update/resources | Firmware identity/size/header checked before staging, exact copies and close failures; fixed vendor image preserved | Updater source unavailable; acceptance of the larger split image unknown |

Current linked size: **3,982,680 bytes**. EWRAM heap reservation: **15,760 bytes**; user stack reserve: **12 KiB**; IWRAM overlay to IRQ SP: **1,680 bytes**. NES patch bytes are unchanged, now const in ROM instead of occupying 832 bytes of IWRAM. No hot renderer tables were moved.

Run `bash scripts/verify-blue-port.sh` for build, protected layout, packaging, native/NOR/metadata/IRQ/folder/update/boot tests and all 24 shared software/UI suites. See [target packaging instructions](../../targets/blue/README.md). No separately uploaded source archive is needed.

Full aggregate validation passed. [BLUE Test 1](https://github.com/voluptuary/ezfode-dssmm-kernel/releases/tag/v8.3.1-blue-test1) is published as a hardware-untested prerelease at 0b29e90; downloaded binary SHA256 verified. Next: complete remaining Omega backports, then audit all three branches. Relevant DE findings remain in [the review queue](EZFODE_REVIEW_QUEUE.md). Do not merge target hardware code across branches. No stable DE release change is part of this port.

## Historical baseline investigation

## Baseline and isolation

Active branch: ezfblue only. Vendor baseline: ezflash-team/blue-kernel 109f1a050d9c35008379ce61ae40d05dc528c614 (unchanged after reference refresh). DE remains origin/ezfode 5f700c9a89768309f06993984bb6c3a3190d04ef. Original Omega test is separately published as v8.3.1-omega-test1. Neither branch is to be merged here wholesale.

The BLUE source is the vendor's classic menu, not DS Style. Establish it as the hardware build baseline under targets/blue before adapting the retained root source/images as a DS Style donor. The root build must delegate only to BLUE; inherited DE host scripts do not validate this target. No hardware-tested BLUE firmware or release exists.

## Verified hardware/build inventory

- Vendor source/ez_define.h: PSRAM aperture 0x08800000; flash aperture 0x09000000; first game allocation 0x00800000; NOR end 0x08000000; 960-byte NOR allocation map, separate metadata blocks at 0x780000/0x7A0000/0x7C0000/0x7E0000. These differ from Original Omega and DE metadata policies.
- Vendor ezbluekernel.c: native ROM acceptance up to 64 MiB; PSRAM selectors advance 0x1000 per 8 MiB; game-mode page 0x400; separate GBApatch_DDR path. Do not replace these with Original Omega 32 MiB assumptions.
- Vendor Link_kernel_image.exe is a PE x86-64 executable. Static disassembly of main (0x4015f7..0x401a3e) shows raw code written at offset zero, resource image.bin written at 0x100000, leaving a zero-filled filesystem hole. No checksum/header transform. It does not guard against code overlapping resources. Portable packer must reject code above 1 MiB and preserve image bytes.
- image.bin is 1,075,676 bytes, SHA-256 be609b61939eaa19b97b81345f8e44e7c97b819fb76d230553d40f2d28688644. Firmware is at resource offset 0x95000 (ROM 0x08195000), length 0x716a0; Python zlib CRC32 exactly matches source constant 0x22DD74FA. Vendor resource prefix is also the old GUI artwork.
- Linked image is 2,124,252 bytes. This is the observed vendor recipe, not proof of bootloader compatibility with arbitrary larger/relocated custom images. Existing ~2 MiB DS Style code would overlap the fixed vendor resource location. Decide a resource/firmware relocation strategy from source/hardware evidence before linking DS Style.
- Current devkitARM rejects vendor missing prototypes, a byte-buffer/word-pointer parameter mismatch, and a lowercase HZK12 include on Linux. Minimal declaration/case corrections build the baseline at 703,752 raw bytes. No flash command sequence, firmware bytes, or reset assembly changed. Inherited warnings (including overlapping sprintf directory appends) require review, not suppression.

## Feature matrix / next sequence

| Area | Current status |
| --- | --- |
| Cartridge/SD/flash/reset/firmware | Vendor baseline; no hardware validation |
| Portable BLUE resource linking | Implemented exact layout with 1 MiB overlap guard, pinned resource hash/firmware CRC, exclusive output; four host test groups pass |
| DS Style launcher/themes/navigation | Root source/images retained as donor; not yet built for BLUE |
| Name-matched/transparent artwork and cache | Pending launcher integration; do not claim present |
| Saves/settings/remembered cheats | Vendor behavior initially; transactional/persistence changes pending target review |
| Loader safeguards | Native file loader exact-read/size/window/padding checks and caller abort ported and host-tested. Plugin, header, trim and hardware/FAT-offload paths still need review |
| NOR catalog/allocation/patching | Vendor format retained; inventory and bounded tests needed before Merkin changes |
| Hardware verification | No reports available; boot/save/NOR/64 MiB/plugin matrix all unverified |

Next: commit inventory, introduce isolated vendor build and tested portable packer, then port native loader exact-I/O/bounds checks with failure propagation. Review SD directory append overflow and catalog enumeration separately. DE shared findings must be reproduced against current ezfode before queueing; do not assume vendor bugs exist there.

Isolated build checkpoint: root make now delegates only to targets/blue. Full target build succeeds at 703,752 bytes; four packaging test groups pass. Raw ELF symbols: EWRAM heap start/end 0x0203E36C/0x02040000 (7,316 bytes); IWRAM overlay end 0x03007080 to user SP 0x03007F00 (3,712-byte gap). These are static reservations, not measured runtime peaks. Eight inherited warning diagnostics remain, including directory sprintf overlap/overflow; none suppressed. Windows batch changes are statically reviewed only. No packaged update distributed.

## First Merkin safety backport validated

Native Loadfile2PSRAM now checks the full exFAT size before narrowing (1..64 MiB), seek/read/close results, pads the final 128 KiB block with FF, restores selector zero on failure and success, and preserves BLUE 8 MiB windows/0x1000 page stride and streaming PatchInternal. Its actual caller aborts before Make_pat_file/further patching/boot on failure. Host tests compile the production function and simulate all eight banks, one-byte/tail/full-limit inputs, >4 GiB narrowing cases, short/failed reads on blocks 1/65, and open/seek/close errors. The pre-fix baseline fails the deterministic-tail assertion; corrected function passes. Caller ordering is checked statically, not by executing the full target menu.

Clean build and bash scripts/verify-blue-port.sh pass: raw code 703,912 bytes, 344,664 bytes before the fixed resource boundary; raw SHA-256 76731cf06d11e8ce84bcba787f22139c29bd69caa913ce6e1fcbf4fe3b386f5f. Resource recipe verified in memory, no installable candidate distributed. EWRAM heap 7,316; IWRAM-to-user-SP gap 3,664 bytes. Eight inherited warnings persist; no runtime stack/heap or hardware claim.

Correction to initial baseline checkpoint: the temporary vendor build succeeded, but the first repository build's final size-guard command failed because its path was relative to the recursive build directory. That failure was initially overlooked in the baseline checkpoint. Corrected to an output-relative absolute path, added Makefile/script dependencies and .DELETE_ON_ERROR, and checked the code boundary on every root make. A subsequent clean build and aggregate verifier succeeded end to end. Baseline import retains vendor whitespace; full imported-range diff --check reported inherited whitespace. Focused new changes pass diff --check.

## Exact continuation

1. Read AGENTS/HANDOFF/PROGRESS and this status. Confirm branch ezfblue and remote ezfode still 5f700c9. Baseline and packaging commit is 2a3d1ca; native-loader checkpoint follows it. Run bash scripts/verify-blue-port.sh after relevant changes.
2. Design DS Style resource/firmware placement before copying the launcher: compare current ezfode 5f700c9 source/ezkernelnew.c, source/ez_define.h, source/fw.h and asset footprint with BLUE's 1 MiB resource split and firmware 0x08195000. Root donor is an older inherited snapshot, not all latest DE changes. Determine whether fixed-address firmware can be preserved with code/asset separation or safely relocated; source-level packer layout alone does not prove bootloader flexibility. Keep vendor firmware CRC/protocols intact.
3. Integrate DS Style frontend against BLUE-specific NOR catalog/allocation, page 0x400 native launch, GBApatch_DDR, settings metadata and firmware APIs. Do not copy DE flash polling/timeouts or NOR path metadata sectors by assumption. Add a per-feature matrix and target regressions before claiming artwork/UI parity.
4. Review vendor ezbluekernel.c folder appends (overlapping sprintf), header/file-size probes, addon trim reads and LoadEMU2PSRAM, plus NOR allocation/catalog functions. Native checked loader does not make these paths safe automatically. Test each narrow correction and checkpoint it.
5. Keep docs/porting/EZFODE_REVIEW_QUEUE.md updated for verified shared DE findings. Existing six entries were established on the unchanged DE commit during Omega work; vendor-only loader/path issues are not new DE bugs. No stable DE changes/merges/releases during BLUE work.
6. Hardware verification remains outstanding for BLUE boot/SD, game saves/settings, plugins, cheats/RTS, 32/64 MiB games and NOR install/delete/format. Do not publish raw baseline as a finished DS Style build.

## Fixed-resource layout proof recovered

Run `python3 scripts/probe-blue-layout.py --report docs/porting/BLUE_LAYOUT_MEASUREMENTS.json`. It exports current DE into a temporary directory, builds before/after a ROM-only relink, checks every loaded section against the resource/metadata reservations, compares 103 artwork/font/audio objects byte-for-byte, and checks unchanged RAM sections. The replay passed at 3,978,912 bytes with 3,885,408 before metadata. Layout: executable prefix below 0x08100000; vendor image unchanged at 0x08100000..0x082069DC; DSSMM readonly data from 0x08207000; all load sections end below 0x08780000. No compression or extra RAM is required.

This establishes non-overlap and feature-asset capacity, not updater size acceptance or functional BLUE behavior. The vendor repository includes no updater bootloader source. No update artifact is emitted by the probe. Next: use this split for the actual BLUE frontend while preserving BLUE-specific backend behavior and metadata.
