## 2026-11-12 — Initial SuperFW import

- Pinned upstream default-branch snapshot `dabbf4909f85e556f7a12972cc8a7b94fc54ac51` from `https://github.com/davidgfnet/superfw`. Kept the upstream tree intact and isolated beneath this directory. Read `docs/UPSTREAM.md` for provenance and license notes.
- The shared registry, root dispatch, CI and merge/sparse scope wiring is in progress. Built `sd`, `lite` and `chis` with devkitARM 16.1. The upstream cimpl, util, UTF utility, cheats and CRC host test groups pass locally. SHA-256 tests/full coverage verifier need `libssl-dev` and `lcov`, currently unavailable here; CI is configured with those dependencies. `chis` retains upstream unused-variable warning. Exact artifact sizes/hashes and limitations are in `docs/bootstrap-validation.json`. No physical SuperCard validation is claimed. Next: await complete CI, then create/push the new lane and verify all five refs.

- Completion: clean-checkout build and full SuperFW host tests passed on hosted run43263202200. The all-project trunk matrix passed on run43263181698. Initial clean-checkout failure was fixed by tracking the six upstream emulator payloads that root `.gitignore` had hidden. Remote `superfw` branch created; all five branches are synchronized at the integrated tree. No physical hardware validation.

## 2026-11-12 — SuperChis Prime port planning

Owner identifies physical test hardware as SuperChis Prime v1.0. Added docs/PORTING-PLAN.md: retain native SuperFW chis backend, build DSSMM presentation incrementally, classify transferable features, and gate runtime/NOR/emulator work on physical tests. Exact revision compatibility and recovery path remain unverified. Documentation only; no firmware behavior changed or new hardware tests claimed. Next: review installed firmware/board identity and inventory rendering/assets before first UI slice.

## 2026-11-12 — Designer-thread evidence

Added the owner-provided ChisBread Prime introduction to docs/PORTING-PLAN.md. Refined clock/rumble exclusions, separate NOR/SDRAM qualification, chip/console identification and distinction between original SuperCard HDL and Prime specifications. Designer claims remain separate from local physical evidence. Documentation only; diff whitespace checked.
