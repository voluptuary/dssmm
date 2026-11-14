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
