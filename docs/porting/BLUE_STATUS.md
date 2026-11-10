# BLUE port status

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
| Loader safeguards | Vendor native loader ignores short reads and caller ignores its failure; first narrow backport candidate |
| NOR catalog/allocation/patching | Vendor format retained; inventory and bounded tests needed before Merkin changes |
| Hardware verification | No reports available; boot/save/NOR/64 MiB/plugin matrix all unverified |

Next: commit inventory, introduce isolated vendor build and tested portable packer, then port native loader exact-I/O/bounds checks with failure propagation. Review SD directory append overflow and catalog enumeration separately. DE shared findings must be reproduced against current ezfode before queueing; do not assume vendor bugs exist there.

Isolated build checkpoint: root make now delegates only to targets/blue. Full target build succeeds at 703,752 bytes; four packaging test groups pass. Raw ELF symbols: EWRAM heap start/end 0x0203E36C/0x02040000 (7,316 bytes); IWRAM overlay end 0x03007080 to user SP 0x03007F00 (3,712-byte gap). These are static reservations, not measured runtime peaks. Eight inherited warning diagnostics remain, including directory sprintf overlap/overflow; none suppressed. Windows batch changes are statically reviewed only. No packaged update distributed.
