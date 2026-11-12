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

- Lookup follow-up: existing DE CUSTOM manifest is a runtime directory-scan hash index, not a mandatory on-disk file. DSSCC mirrored output follows relative ROM directories and strips the extension. Record these in docs/DSSCC-FORMAT-AUDIT.md; remaining fallback/encoding details must be reviewed before implementation. Do not impose a new pack manifest on users.

- Final local results: clean sequential chis/sd/lite builds exited0; sanitized decoder tests, real pinned generator interoperability, theme tests and whitespace checks pass. Existing upstream warnings retained. Evidence: docs/prime-artwork-validation.json. Hosted run43272095902 for source63de698 is still pending at this entry; confirm its result before integration. New module is not called by the browser yet, so these builds do not measure future cache/rendering cost. No physical validation or release.

- Hosted completion: source63de698 passed full SuperFW verifier in https://github.com/voluptuary/dssmm/actions/runs/43272095902 (all board builds, mandatory sanitized artwork/theme tests, full upstream host suite). Earlier pending entry is superseded. Next concrete task remains FatFs/name lookup and a selectable on-screen preview; decoder alone is not end-to-end artwork support.

## 2026-11-12 — First selectable SD-browser artwork preview

- Start cycles List/title/box in the SD browser, with a footer hint; existing UI Save persists artwork=0..2. Default remains List. Art modes narrow list text and show the selected file’s name-matched image. Lookup tries mirrored /SYSTEM/IMGS[2]/... then CUSTOM/<stem>.bmp, rejecting unsafe/truncated paths. No directory scan/manifest requirement.
- Exact FatFs seek/read and checked close feed the decoder. Preview output uses a dedicated9600byte halfword buffer appended to native cartridge SDRAM state (existing14.5MiB bound retained), not an internal RAM image cache. The decoder now requires halfword alignment and writes packed pairs; production compositing does halfword-safe transparent overlays into Mode4. Palette20–235 is uploaded with theme reload. Fixed quantization remains lossy.
- Cache hits/misses by full ROM path and style; invalidate on directory reload, mode/init and action/tab keys that may reuse cartridge RAM. Folder/no-art selections do not display stale images. Browser actions, loading and save protocols are unchanged.
- Added actual adapter/path tests with mocked FatFs failures; extended compositor/canary/bounds/alignment and settings save/reload coverage. Sanitized decoder/adapter tests, theme tests and pinned real DSSCC writer check pass. New code is in sequential board builds; final results follow below. A strict host signedness diagnostic was corrected before passing tests.
- docs/ARTWORK-PREVIEW.md describes paths, controls, limitations. No physical visual/performance/gameplay test yet. Folder/header-code/root/extension/NOR/recent fallbacks and localization remain pending. Next: hardware/emulator screenshot and cache invalidation smoke tests, then expand lookup behavior. Keep journals and native backends intact.

- Repeat host validation encountered an ASan DEADLYSIGNAL startup loop under PIE; terminated that test process and rebuilt host sanitizer executables with -fno-pie -no-pie. All sanitized decoder/adapter/compositor, settings and real-generator checks then passed again. This is a host harness adjustment; keep sanitizer coverage enabled. Final committed-source builds and hosted checks follow.

- Hosted full verification passed for implementation4d72847 (run43280511370) and final source/harness0596247 (run43280669644): all three board builds, sanitized artwork adapter/compositor/decoder tests, settings checks and upstream host coverage suite. Final local chis link reports IWRAM12008bytes and EWRAM235676bytes; this does not measure runtime stack peaks. Local final Lite build is still completing at this entry.

- Completion: all three clean sequential local builds from source0596247 exited0; full hosted verification is green. Exact local artifacts/memory/limitations: docs/prime-preview-validation.json. Evidence-only descendants leave executable source unchanged. No physical testing/release claim. Next: confirm owner firmware/recovery and console, validate List/title/box selection/missing-art/transparent-art/cache return paths on hardware or a suitable cartridge model; then expand folder/code/NOR/recent lookups. Work remains in superfw lane, to be integrated normally through dssmm when appropriate.

## 2026-11-12 — Test 1 publication preparation

Owner explicitly requests end-user root README and a prerelease containing all three SuperFW board builds, with chis for physical testing. Prepare one superfw-test1 release with superfw-chis.fw, superfw-sd.fw and superfw-lite.fw; preserve upstream version0.22 and use tag/source hash to identify the first DSSMM test. Four-section end-user notes and first-test guidance are in docs/releases/superfw-test1.md and docs/ARTWORK-PREVIEW.md. No new physical claim, MiGoomba substitution or OMEGA release change. Next: integrate lane through dssmm, root README/baseline checkpoint, validate final source/builds/CI, publish prerelease without making latest, download/compare assets and record evidence.

- Owner adds end-user artwork creation/install instructions. Added docs/ARTWORK-SETUP.md with actual DSSCC Custom Art/Scan SD Card/Mirrored/Build Pack flow, GBA inclusion checkbox, both sizes, output location, exact SYSTEM layout, Start/UI Save controls and troubleshooting. Linked from preview and release notes. Instructions reviewed against pinned DSSCC38424ce; no firmware changes.

## 2026-11-12 — SuperFW Test 1 published and downloads verified

- Owner-authorized https://github.com/voluptuary/dssmm/releases/tag/superfw-test1 is public, prerelease=true/draft=false/latest=false, annotated tag source8a22da16c2d755de8fc12db7c5482657e6813ecd. Three assets only: superfw-chis.fw (owner’s Prime v1.0 test), superfw-sd.fw, superfw-lite.fw. Native upstream version0.22 is retained; firmware source identifier8a22da16 distinguishes these builds. No physical test is claimed.
- Final local sequential builds and full all-project/lane CI passed. Chis43282667921, trunk43282661424, DE43282666938, OMEGA43282667191, BLUE43282666475. Board IDs, GBA headers, embedded sizes/source hash and full-image seals verified. Draft then public downloads match staged bytes/hashes and GitHub digests. Canonical four-section body matches remote. Evidence: docs/releases/superfw-test1-publication.json.
- All19 prior release metadata/bodies/assets and tag refs verified unchanged. DEv8.4.0 remains latest stable. Root README is end-user focused with correct hardware downloads and a DSSCC artwork workflow. docs/ARTWORK-SETUP.md covers Custom Pack (older name Custom Art), scan actual SD root, Include GBA, both sizes, Mirrored/Custom build, exact SYSTEM folders, Start preview controls and UI Save. No OMEGA source changes.
- Next: owner tests chis. Record console/model/installed firmware/recovery details and observed boot/theme/artwork/save results before fixing or claiming qualification. Keep tag/assets immutable; subsequent evidence-only commits do not change tested firmware inputs. Next development remains full lookup/folder/NOR/recent support and DS-style layout after feedback. Preserve all journals and the user’s untracked devcontainer lock. Integrate this evidence through dssmm and sync lanes.


## 2026-11-12 — Prime v1.0 boot/theme smoke test clarified

Owner corrected the initial report: the stock kernel reports version 0.20, while the Test1 Chis image successfully chain-loads and reports SuperFW 0.22. Manual themes are available. The earlier interpretation of “returned to default loader” was mistaken; remove the failed-boot diagnosis. The user expected a more visibly DSSMM-style menu, but the overall layout is still mostly upstream SuperFW. Record this as successful initial boot/theme smoke testing on one SuperChis Prime v1.0, not full hardware qualification. Gameplay, saves, artwork rendering, NOR, installation, and SD/Lite variants remain untested. Evidence: `docs/prime-test1-hardware-observation.json`. Next work should focus on the DSSMM-style presentation.


## 2026-11-12 — Owner clarification: Test 1 boots successfully

SuperChis Prime v1.0 chain-loads the Test1 Chis image and reports SuperFW 0.22 (stock kernel reports 0.20); manual themes are available. The menu remains mostly upstream SuperFW, and the full DSSMM-style layout is still in progress. This supersedes the immediately preceding mistaken failed-boot interpretation. Only boot/version/theme availability are physically reported; do not infer artwork, gameplay, save, NOR, install, SD or Lite validation. See `docs/prime-test1-hardware-observation.json`.

## 2026-11-12 — Identify DSSMM builds and name themes

Updated the Info page identity to `SuperFW <VERSION_WORD major.minor> (DSSMM)`, retaining the commit identifier and board variant. The displayed version comes from the existing SuperFW `VERSION_WORD` build input (currently 0.22). Replaced numeric theme labels with palette names, removing the visible `< 6 >` while retaining IDs 0–7 and saved settings compatibility. Local theme settings and artwork host checks pass; `git diff --check` passes. Full verifier/build unavailable locally because `arm-none-eabi-gcc` is missing. Next: hosted SuperFW CI, then integrate through dssmm and sync lanes; no release or physical validation implied.

## 2026-11-12 — Display DSSMM build number

Replaced the Info-page short commit hash with `DSSMM Build N`, where `N` comes from `git rev-list --count HEAD` across full shared repository history. The existing firmware metadata still embeds `VERSION_SLUG_WORD` as the upstream commit identifier. CI already fetches full history. `make -n -C superfw BOARD=chis` confirms `-DDSSMM_BUILD_NUMBER="506"` for the current pre-commit checkout; `git diff --check` passes. No local ARM compiler is available, so await hosted matrix to validate all board builds before completion. No release was requested.

## 2026-11-12 — First visible DSSMM browser skin pass

Added a DSSMM wordmark to the tab strip, a full-width active-color selected row, and a framed artwork preview panel. The browser's selection/navigation, filesystem lookup/cache, and SuperFW launch actions are unchanged. Updated `docs/PORTING-PLAN.md`; its earlier “no physical result” status is corrected to the owner-reported boot/theme smoke test. Theme settings and sanitized artwork host tests pass; `git diff --check` passes. Physical visual validation and ARM builds remain outstanding (local devkitARM compiler unavailable). Next: hosted matrix, then inspect screenshot feedback and continue translating DSSMM browser composition; do not call this complete skin parity.
