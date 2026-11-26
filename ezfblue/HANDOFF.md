# ezfblue cold-start handoff

Purpose: current state for the newer OMEGA DE B / BLUE lane. Read root
`AGENTS.md` and [`docs/AGENT-QUICKSTART.md`](../docs/AGENT-QUICKSTART.md) first;
this file replaces the pre-compaction chronological handoff.

## Current state

- Release: **8.4.1 Test 5** (`v8.4.1-blue-test5`, source `9bc24e04`; the
  firmware reports 8.4.2), prerelease, hardware-untested.
- Latest work: alias-free exFAT launch correction using optional aliases,
  canonical paths and recorded directory entries. Five native mGBA launch cases
  and sanitized path helpers pass.
- Image policy: stock-size, firmware-free sealed image. FPGA updates require the
  official BLUE kernel.
- Hardware: untested.

## Next actions

1. Obtain BLUE hardware coverage when the owner can test.
2. Preserve the sealed stock-size image and official-FPGA-update requirement.
3. Keep save/transfer/reset protocols separate from DE and Original OMEGA.

## Build and verify

```sh
make ezfblue
make verify-ezfblue
```

Output is log-first: the full transcript is in repository-root
`build/verify-ezfblue.log`; `DSSMM_VERBOSE=1` streams it.

## Evidence

- `docs/blue-sd-launch-validation.json` — SD-launch and native cases.
- `docs/native-menu-preview-validation.json` — menu preview captures.
- Root `docs/current-builds.md` — validated checkpoints.
- Pre-compaction journals:
  `docs/history/journal-pre-compaction-2026-11-26/`.
