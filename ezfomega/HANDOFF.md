# ezfomega cold-start handoff

Purpose: current state for the Original EZ-FLASH OMEGA lane. Read root
`AGENTS.md` and [`docs/AGENT-QUICKSTART.md`](../docs/AGENT-QUICKSTART.md) first;
this file replaces the pre-compaction chronological handoff.

## Current state

- Release: **8.4.1 Test 5** (`v8.4.1-omega-test5`, source `29bbb055`),
  prerelease, hardware-untested.
- Latest work: clipped alias-free SD-launch correction restored full-name
  lookup with paired-dirent offset checks. Five native mGBA cases, sanitized
  production-helper negatives and the full verifier passed; production image is
  2,055,088 B with 19,368 B heap and 8,312 B static gap.
- Hardware: untested. Native fixtures mock storage, save, transfer, patch and
  reset; they are not physical qualification.

## Next actions

1. Obtain Original OMEGA hardware coverage when the owner can test.
2. Keep launch/path behavior aligned with DE without copying its driver.
3. Add focused fixtures for any regression; do not claim hardware results.

## Build and verify

```sh
make ezfomega
make verify-ezfomega
```

Output is log-first: the full transcript is in repository-root
`build/verify-ezfomega.log`; `DSSMM_VERBOSE=1` streams it.

## Evidence

- `docs/sd-launch-validation.json` — SD-launch and native cases.
- `docs/native-menu-preview-validation.json` — menu preview captures.
- Root `docs/current-builds.md` — validated checkpoints.
- Pre-compaction journals:
  `docs/history/journal-pre-compaction-2026-11-26/`.
