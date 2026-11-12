## 2026-11-12 — Initial SuperFW import

- Pinned upstream default-branch snapshot `dabbf4909f85e556f7a12972cc8a7b94fc54ac51` from `https://github.com/davidgfnet/superfw`. Kept the upstream tree intact and isolated beneath this directory. Read `docs/UPSTREAM.md` for provenance and license notes.
- The shared registry, root dispatch, CI and merge/sparse scope wiring is in progress. Built `sd`, `lite` and `chis` with devkitARM 16.1. The upstream cimpl, util, UTF utility, cheats and CRC host test groups pass locally. SHA-256 tests/full coverage verifier need `libssl-dev` and `lcov`, currently unavailable here; CI is configured with those dependencies. `chis` retains upstream unused-variable warning. Exact artifact sizes/hashes and limitations are in `docs/bootstrap-validation.json`. No physical SuperCard validation is claimed. Next: await complete CI, then create/push the new lane and verify all five refs.

- Completion: clean-checkout build and full SuperFW host tests passed on hosted run43263202200. The all-project trunk matrix passed on run43263181698. Initial clean-checkout failure was fixed by tracking the six upstream emulator payloads that root `.gitignore` had hidden. Remote `superfw` branch created; all five branches are synchronized at the integrated tree. No physical hardware validation.

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

## 2026-11-12 — Independent manual accents implementation checkpoint

- Added Default/Blue/Green/Orange/Purple selection accents, with bounded `accent` settings parsing and serialization. Default retains exact existing palette values; overrides affect only selection/highlight roles. Main and in-game palettes use the same resolver. Theme changes retain the independent accent choice.
- UI settings now has six rows spaced 18 pixels apart and the existing Save button. New accent labels are English pending the later localization pass, like the initial Light/Dark labels. Visual fitting/contrast remains physically untested.
- Expanded actual-settings host tests: theme/accent bounds, legacy/default palette equivalence, text/background preservation, fallback, and serialized save/reload round trip with mocked filesystem writes. Host variants pass. Sequential all-board builds are underway; final results will be appended before handoff. Full local coverage remains unavailable without lcov.
- Next: pin/audit DSSCC generator output and DSSMM artwork lookup rules; implement bounded external name-matched artwork loading before restructuring the browser. Do not copy another model’s hardware drivers or assume emulator/NOR save compatibility. Owner’s Prime firmware/recovery/console details still needed before hardware testing. Preserve all historical journals.

- Hosted checkpoint: ed92ca1 passed full SuperFW verification (three board builds, theme settings tests and complete upstream host coverage suite), run https://github.com/voluptuary/dssmm/actions/runs/43271213482 . Prior palette checkpoint 32c3009 also passed run43270750833. Initial DSSCC format findings are pinned in docs/DSSCC-FORMAT-AUDIT.md: top-down GBA RGB555 BMP, DST1 transparency marker, indexed-frame rendering consideration. Full lookup/manifest audit and artwork implementation remain next.

- Final local checkpoint: sequential chis/sd/lite builds all exited 0, actual-settings host tests and whitespace checks passed. Exact local artifacts: docs/prime-accent-validation.json. No physical test or release. Kernel code remains ed92ca1; subsequent changes record evidence only. Continue on superfw; integrate through dssmm when ready, preserving all four project trees.

## 2026-11-12 — Bounded DSSCC BMP decoder implementation

- Added src/artwork.c/.h: exact-read callback, strict supported BMP geometry/header/size checks, RGB555 conversion to a fixed 216-color palette at indices20–235, and distinct transparent sentinel0. Scratch row240bytes; caller output at most9600bytes. No static cache, file lookup, palette upload or browser display yet. Failed decode invalidates output; never show partial results. Palette conversion is lossy.
- Sanitized actual-decoder tests pass for sizes, transparency, RGB ordering, canaries, malformed fields/offsets, capacity and read failures. Real pinned DSSCC writer interoperability passes both sizes and marker modes through optional tests/test-dsscc-writer.py (external checkout, no network/Pillow). Theme regression tests pass. All-board sequential builds started; append final status below. scripts/verify.sh includes the mandatory decoder tests.
- Next: finish lookup/manifest audit; exact FatFs adapter; deliberate bounded cache lifetime; selectable List + Art rendering using safe halfword VRAM writes. See docs/DSSCC-FORMAT-AUDIT.md. Physical testing and full pack support remain pending. Preserve upstream backends and all journals.
