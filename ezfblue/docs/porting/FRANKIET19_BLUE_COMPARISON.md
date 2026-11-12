# FrankieT19 BLUE port comparison — 2026-11-12

Reviewed [FrankieT19/omega-blue-ds-style-kernel](https://github.com/FrankieT19/omega-blue-ds-style-kernel)
at [`08ab566c420641b0f33ffb914e37c35a7e32857a`](https://github.com/FrankieT19/omega-blue-ds-style-kernel/tree/08ab566c420641b0f33ffb914e37c35a7e32857a)
(DS Style 8.1 Blue Test 5), against DSSMM `v8.4.0-blue-test3`.
Both ports start from EZ-FLASH BLUE `109f1a0`. Our DS Style fork lineage and
FrankieT19's authorship remain credited in [CREDITS.md](../../CREDITS.md).

## Hardware evidence

The pinned upstream README explicitly says Test 5 has not been verified on a
physical EZODE-B. Its releases describe hardware **test builds** and ask for
results; no issues or hardware-result reports were present when reviewed.
A request to test hardware is not a successful hardware report. Neither that
upstream port nor this combined development build is hardware-qualified by this
review. The existing public DSSMM BLUE Test 3 remains an unmodified prerelease.

## Improvements adapted into DSSMM development code

| Area | Finding and resulting behavior | Source of comparison |
| --- | --- | --- |
| Update size and firmware | Removed the live updater and firmware address constants. Link live code/assets contiguously and pad to the vendor's 2,124,252-byte update size. Retain the historical vendor resource file for baseline tests, but omit it and its FPGA payload from the current update. Use the official BLUE kernel for firmware updates. | Test 5, `08ab566` |
| Complete image integrity | DSSMM's new `DSSMBL1` seal checks CRC32 plus its complement across the complete image before style staging. Checks include exact size, model header and header checksum. Earlier validation checked only the firmware region and missed changed code/artwork. CRC is an integrity check, not an authenticity signature. | Test 3/Test 5 style validation, `3c55cc9` / `08ab566` |
| Pending battery save | A pending save flag blocks another save load and manual FRAM operations. NOR launch rejects it before preparing launch state. A failed backup can no longer be bypassed by launching another title and overwriting the pending FRAM data. | Test 3, `3c55cc9` |
| NOR browsing and allocations | Removed automatic metadata writes on startup/empty browsing. Only an entirely erased catalog **and** map are normalised in RAM. Writes and selected deletion require agreement between validated entries and the allocation map; orphan reservations are not silently reclaimed. Sort valid entries by physical ROM page; manual FRAM access requires the first physical allocation. Existing readback and reserve/publish/release safeguards remain. | Test 3/Test 4 NOR checks, `3c55cc9` / `bc4418c` |
| NOR add-on extent | Reserve through the end of the patch, including a patch starting at the next block. Reject misaligned offsets, patches crossing a 128 KiB programming block, and patches beyond their 32 MiB execution window. Retain the existing 256 KiB allocation policy. | Test 3 write plan, `3c55cc9` |
| Mode B interoperability | Store the launcher preference in tagged settings word 65 and leave stock Mode B word 15 as its ordinary value. Read the old packed high byte when migrating older DSSMM settings. No extra flash sector is allocated. | Test 4, `bc4418c` |
| SD save states | Stage the exact 448 KiB BLUE state in DDR page `0x6000` at aperture offset `0x20000`, matching BLUE's existing in-game assembly. The old inherited loader wrote older-DE SRAM banks instead. Require the vendor state signature, or a completely all-FF newly allocated empty state, and propagate read/seek/close/FAT errors. Only `FR_NO_FILE` permits exclusive creation; incomplete new states are removed. | BLUE RTS loader and Test 3 validation, `c9af5b0` / `3c55cc9`, cross-checked against our vendor assembly |
| Persistent NOR state safety | Block writing new NOR add-ons with save states, and block launching existing entries marked with that combination. Their embedded SD sector tables can become stale. Keep the entries and files intact. Use clean NOR entries or disable save states before writing add-ons; use the original kernel for an existing entry. | Upstream's explicitly guarded NOR states and later separate runtime, `1284b8b` / `c9af5b0` / `bc4418c` |

These are independently implemented changes to DSSMM's existing vendor-derived
code and build tools, informed by FrankieT19's review/design work. No Slot.
Advance runtime, trampoline, emulator patch or GPL source was copied. The
upstream [NOTICE](https://github.com/FrankieT19/omega-blue-ds-style-kernel/blob/08ab566c420641b0f33ffb914e37c35a7e32857a/NOTICE.md)
identifies its combined Slot-derived runtime as GPL-3.0-or-later; its notices and
original authorship are not replaced by a blanket attribution to this project.

## Compared features retained or deferred

| Upstream work | Decision |
| --- | --- |
| Original bitmap assets and DMA restoration | Retain our existing Merkin Mix artwork, transparency, caches and DMA paths, covered by differential tests. A wholesale replacement would discard independently tested features. Compact linking does not change the live artwork bytes. |
| Slot-derived warm-return, boot-entry trampoline and modified Jagoomba | Do not transplant them into our different CRT, reset/stack handoff and newly validated MiGoomba payload. They require a coordinated runtime integration, licensing/provenance work and additional compatibility tests. Current boot safety tests remain. |
| BLUE text viewer and plugin namespace | Keep our existing plugin interfaces. Upstream's viewer and return path are coupled to its runtime; merely copying the binary would not validate return behavior. |
| Native NES fallback | Keep the current tested external NES/plugin path; upstream's embedded fallback requires its resource/payload and launch-contract review. No claim of newly added native NES support. |
| Large-ROM add-ons | Keep native 64 MiB/5G clean loading and our existing bounded add-on path. Upstream's large-ROM padding/scratch checks depend on its matching DDR/add-on patch. Do not infer compatible 64 MiB add-ons from a new allocator alone. |
| Persistent and legacy NOR states | Defer the coupled patch/cookie/warm-return/sector-map integration. Add the explicit safe refusal described above rather than launch a known-unverified persistent SD sector map. |
| NOR flash drivers | Retain our checked reads, programming readback and IRQ/bank restoration. Upstream's simpler deletion/write routines do not replace these safeguards or establish physical timing limits. |

## Validation and limits

See [frankiet19-blue-review.json](frankiet19-blue-review.json) for exact commits,
build sizes, asset comparison and completed test results. The new development
version is `8.4.1-blue-dev`; it is not a published release.

The focused fixture fails against `v8.4.0-blue-test3` because a pending save is
not blocked, then passes against this implementation. Tests exercise the actual
production save and RTS functions, injected file errors, DDR staging, exact
kernel-byte corruption, allocation/map disagreement, factory-erased handling,
physical NOR ordering and patch bounds. Historical vendor packing tests remain.
The full BLUE verifier includes the existing 24 UI/software suites, loading,
NOR, IRQ/boot and linked layout checks.

Static capacity and mocks do not prove updater acceptance, cold/warm boot,
cartridge saves, physical NOR behavior, game compatibility or runtime stack
peaks. Firmware upgrades must use the official BLUE kernel. Existing SD rename
and NOR catalog power-loss limitations remain; avoid interruption during writes.

For a local development image, from the Git root:

```sh
make verify-ezfblue
python3 ezfblue/scripts/pack-blue-kernel.py \
  --code ezfblue/targets/blue/ezbluekernel-development.gba \
  --elf ezfblue/targets/blue/ezbluekernel-development.elf \
  --output ezfblue/build-blue-review/ezbluekernel.bin
```

Create the ignored output directory first. The packer refuses existing outputs.
The raw `.gba` omits padding and the seal and is not an installable update.
Load style accepts the new sealed DSSMM BLUE format; old firmware-bearing
DSSMM update files and upstream's different seal are intentionally rejected by
that staging path. Published releases remain available unchanged for rollback.
