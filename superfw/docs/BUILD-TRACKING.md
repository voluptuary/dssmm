# DSSMM build tracking

The displayed DSSMM number is the complete Git revision count. Commit production
changes before making a candidate. All three boards built at the same commit use
the same number; rerunning a build does not allocate a different number.

`scripts/build.sh` captures that number and passes it explicitly to Make. Each
board image gets an ignored `build/<board>/build-info.json` containing the number,
commit, dirty status, project file fingerprint and firmware hash. Preserve those
records with validation evidence before cleaning output. A successful build does
not qualify runtime or hardware behavior. Record hardware reports by board,
build number, displayed hash and image hash when available.

## Reported hardware candidates

- Build 534, displayed hash `126ae458`: owner reports carousel first-file edge
  crash, no Title/Box carousel artwork, and severe slowdown with artwork enabled;
  artwork List mode improves responsiveness. Commit 126ae458 has no carousel
  implementation, so this candidate contained uncommitted source. Its complete
  source/image identity cannot be reconstructed from the displayed hash alone.
- Edge crash investigation: current carousel Left navigation used unsigned step
  subtraction at index zero. The production-navigation regression fails on that
  code and passes after signed arithmetic. Hardware retest still required.
- Missing artwork and slow carousel rendering remain open hardware findings;
  newer cache/renderer changes are not established fixes until retested.

Run `python3 superfw/scripts/check-build-set.py` from the repository root before
using a three-board set as a candidate. It requires clean matching manifests,
identical source commit/build number/project fingerprint, matching image hashes,
valid GBA header checksums, embedded sizes, and full firmware seals. It emits a
JSON report on success. This is integrity/identity validation; it does not replace
build logs, tests, or runtime/hardware qualification. It correctly rejects the
existing diagnostic set because Lite was built with modified source.

## Build 553 result and Build 561 retest

Owner confirms Chis553 no longer crashes at the first entry and Box art works
in List view; carousel artwork is absent. Build561 (76b7a493) moves byte-written
carousel metadata from cartridge SDRAM into GBA RAM. Its ARM bus regression
passes and553 fails. All three clean board builds and hosted CI pass; see
build-561-validation.json. Chis561 was delivered as a local hardware-test image.
Physical artwork/performance confirmation is still pending.

## Current local candidate: Build582

Source842bad386c5c3f5616a4b01eaa4f5e3f8596c5c2, displayed hash842bad38.
All three clean variants share fingerprint and pass integrity/memory gates and
hosted verification. Chis582 is the current hardware-test image; earlier local
images were superseded after their evidence was retained. Build581 was not
distributed after the BMP-tail audit. See build-582-validation.json. Physical
Header Title matching, speed and in-game save/restore qualification remain open.

## Current local candidate: Build588

Source2a150a379d28bac81418ece349a0bf343b963b15, hash2a150a37.
All three clean variants pass integrity/memory and hosted gates. Chis588 in
root hardware-test supersedes585 locally. See build-588-validation.json for
local ASAN startup limitation and successful full non-PIE sanitizer rerun.
Owner physically confirms585 header art/cache; new directory CUSTOM and font
remain unqualified on hardware.
