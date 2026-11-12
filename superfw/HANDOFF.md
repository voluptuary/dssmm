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

## 2026-11-12 — DSSCC artwork compatibility target

Owner requests retaining name-matched packs built with voluptuary/dsscc. Added its documented dimensions/layout/BMP format to docs/PORTING-PLAN.md, plus source-format audit and real-pack/physical rendering acceptance gates. Implementation remains pending; no generator or firmware code changed. Documentation whitespace check passed.

## 2026-11-12 — First Prime implementation: manual palette foundation

- Owner authorized starting implementation with the easiest parts. Added original Light/Dark palettes (IDs 6/7), named UI choices and Light default for new chis configurations. Existing numeric IDs 0–5 and persisted choices remain; sd/lite defaults remain upstream. Native theme application also supplies the in-game palette. No automatic themes, imported graphics or hardware/backend changes.
- Added scripts/test-theme-settings.sh and tests/theme-settings-test.c, exercising the actual settings parser for legacy/new IDs, invalid values, per-board defaults and readable foreground/background separation. Both host variants pass; integrated in the project verifier. Full coverage suite remains unavailable locally because lcov is absent; do not count prior hosted results as validation of this new source.
- An accidental overlap of chis/sd builds invalidated the initial build result; stopped the overlap and reran clean builds sequentially. Always serialize variants because upstream build intermediates are shared.
- Next: independent accent control, then DSSCC format/lookup audit and bounded artwork reader before List + Art. Existing menu geometry remains upstream; this is not a completed DS-style skin. Physical theme contrast, persistence and gameplay remain untested. Get owner firmware/recovery details before hardware candidate instructions. Preserve all source/credits and the untouched pre-existing devcontainer lock.

- Final local result: clean sequential chis, sd and lite builds all exited 0; theme parser/default tests and git diff --check pass. Retained upstream RWX linker/unused-variable warnings. Artifact hashes/sizes: docs/prime-theme-validation.json. Push this checkpoint to superfw; hosted full verifier still needs confirmation for this commit.
