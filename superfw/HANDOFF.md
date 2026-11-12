## 2026-11-12 — SuperFW project bootstrapped

- Added an isolated `superfw/` project from upstream `davidgfnet/superfw` commit `dabbf4909f85e556f7a12972cc8a7b94fc54ac51`. Preserved upstream source, assets, tests, nested workflows and notices; included GPL-3.0 text and provenance. The project covers upstream `sd`, `lite` and `chis` variants. No SuperFW firmware has yet been built or tested in this repository, and no physical validation is claimed.
- Upstream files were imported and the shared registry/dispatch/CI/isolation setup is now being wired on `dssmm`. Remaining: collect configured CI results for the complete upstream test suite, then create/push the `superfw` lane and verify all five branch refs. Keep all target-specific follow-up inside this directory.


## 2026-11-12 — Clean-checkout SuperFW validation complete

- Root ignore rules initially omitted six upstream emulator payloads; the first clean-checkout CI run failed. Added those exact upstream-tracked assets under `emu/`, added the root ignore exception, and refreshed the source baseline.
- Corrected hosted SuperFW verification passed in run [43263202200](https://github.com/voluptuary/dssmm/actions/runs/43263202200): all three board builds plus the upstream coverage-enabled host tests. The full dssmm matrix, including SuperFW and all OMEGA projects, passed in run [43263181698](https://github.com/voluptuary/dssmm/actions/runs/43263181698). Local policy/merge/snapshot/site checks and three board builds also pass; five local host groups passed, while hosted CI covered the complete suite. `chis` has an upstream unused-variable warning. No physical SuperCard hardware validation was performed.
- Branch `superfw` is live on origin and tracks the same integrated tree. Continue future SuperFW work only in this directory on its matching lane, then merge through dssmm.

## 2026-11-12 — SuperChis Prime port planning

Owner identifies physical test hardware as SuperChis Prime v1.0. Added docs/PORTING-PLAN.md: retain native SuperFW chis backend, build DSSMM presentation incrementally, classify transferable features, and gate runtime/NOR/emulator work on physical tests. Exact revision compatibility and recovery path remain unverified. Documentation only; no firmware behavior changed or new hardware tests claimed. Next: review installed firmware/board identity and inventory rendering/assets before first UI slice.

## 2026-11-12 — Designer-thread evidence

Added the owner-provided ChisBread Prime introduction to docs/PORTING-PLAN.md. Refined clock/rumble exclusions, separate NOR/SDRAM qualification, chip/console identification and distinction between original SuperCard HDL and Prime specifications. Designer claims remain separate from local physical evidence. Documentation only; diff whitespace checked.

## 2026-11-12 — Unsupported-feature scope decision

Owner authorizes dropping features unsupported by Prime hardware, including automatic themes. Updated docs/PORTING-PLAN.md: manual Light/Dark/accent controls remain; absent hardware controls are omitted. Existing game software-RTC support is a separate native capability. Documentation only; git diff --check passed.
