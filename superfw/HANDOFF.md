# superfw cold-start handoff

Purpose: current state for the independent SuperCard SD/Lite/Chis lane. Read
root `AGENTS.md` and [`docs/AGENT-QUICKSTART.md`](../docs/AGENT-QUICKSTART.md)
first; this file replaces the pre-compaction chronological handoff.

## Current state

- Release: **Build 822** (`superfw-build822`, source `2f73b53b`) for all three
  boards; public and software-qualified. Chis owner observations cover boot,
  Atari plugin load/Exit and PCE Exit only; SD/Lite and full gameplay are
  unqualified.
- Local checkpoint: **Build 827** (`db4e640`), unpublished. It adds the leased
  `menu_workspace.c/.h` union of the artwork hot copy and settings snapshot,
  recovering about 8,144 B of main EWRAM (Chis 248,504 B used) while keeping
  five artwork slots, 8,192 B settings capacity and the 16 KiB stack floor.
- Build 822 remains the public and hardware-test image; Build 827 is a software
  checkpoint awaiting full gates and integration.

## Next actions

1. Finish validating and integrate Build 827 through `dssmm`, then sync back.
2. Implement the owner-requested 19px SuperCard top bar and matching colors
   within the validated memory budget.
3. Continue parity: PCE icon/return polish, plugin Exit paths, settings
   layout/persistence, NOR/artwork edges, then SD/Lite behavior.
4. Do not publish Build 827 or recreate Build 640/Test 6 without an explicit
   owner request.

## Build and verify

Build board variants **sequentially** because they share intermediates.

```sh
make superfw BOARD=sd
make superfw BOARD=lite
make superfw BOARD=chis
make verify-superfw
```

Output is log-first: full transcripts are in repository-root
`build/build-superfw-<board>.log` and `build/verify-superfw.log`;
`DSSMM_VERBOSE=1` streams them.

## Evidence

- `docs/build-822-validation.json` — published checkpoint.
- `docs/build-827-validation.json` — workspace/RAM checkpoint and limits.
- `docs/releases/superfw-build822-publication.json` — publication record.
- `docs/BUILD-TRACKING.md`, `docs/UI-PARITY.md`, `docs/PLUGIN-COMPATIBILITY.md`.
- Pre-compaction journals:
  `docs/history/journal-pre-compaction-2026-11-26/`.
