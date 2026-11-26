# ezfomegade cold-start handoff

Purpose: current state for the OMEGA Definitive Edition Rev. A/B lane. Read root
`AGENTS.md` and [`docs/AGENT-QUICKSTART.md`](../docs/AGENT-QUICKSTART.md) first;
this file replaces the pre-compaction chronological handoff.

## Current state

- Release: **8.4.1** (`v8.4.1`, source `a948b936`), stable and GitHub latest.
- Latest work: optional-alias and clipped alias-free SD-launch regression
  software-reproduced and fixed. Full DE verifier passed (43 host scripts, five
  native mGBA launch cases, production build, browser and memory checks).
- Hardware: affected-card physical confirmation for the SD-launch fix is still
  pending. Host/emulator evidence is not hardware validation.

## Next actions

1. Take physical confirmation of the SD-launch fix when the owner can test.
2. Keep DE buildable and reproducible; no new release without owner request.
3. Fix regressions with focused host/native fixtures, not broad refactors.

## Build and verify

```sh
make ezfomegade
make verify-ezfomegade
```

Output is log-first: the full transcript is in repository-root
`build/verify-ezfomegade.log`; `DSSMM_VERBOSE=1` streams it.

## Evidence

- `docs/sd-launch-validation.json` — SD-launch and native cases.
- `docs/native-menu-preview-validation.json` — menu preview captures.
- Root `docs/current-builds.md` — validated checkpoints.
- Pre-compaction journals:
  `docs/history/journal-pre-compaction-2026-11-26/`.
