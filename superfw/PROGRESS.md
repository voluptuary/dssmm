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

- Info page now reports `SuperFW 0.22 (DSSMM)` from the compile-time `VERSION_WORD`, so the text follows whichever SuperFW version is configured at build time. The short source build identifier and SD/Lite/Chis variant remain visible.
- UI theme choices now use palette names (White, Orange, Blue, Green, Purple, Stock Dark, Light, Dark) instead of numbers. This removes the confusing visible “6” option without renumbering or invalidating persisted IDs 0–7.
- `scripts/test-theme-settings.sh` and `scripts/test-artwork.sh` pass; `git diff --check` passes. `scripts/verify.sh` cannot run in this container because `arm-none-eabi-gcc` is unavailable. Firmware builds and hosted CI remain pending.

## 2026-11-12 — DSSMM build number replaces displayed hash

- Info page now shows `DSSMM Build N` from a compile-time `DSSMM_BUILD_NUMBER`, defaulting to the count of commits reachable from HEAD in the shared repository. This avoids showing a raw hash and gives future integrated builds a monotonic repository build number. CI uses full-history checkout; reproducible builds may set the number explicitly. `VERSION_SLUG_WORD` remains unchanged in firmware metadata.
- Dry-run Chis build confirms the compiler receives build number 506 at this pre-commit checkpoint. `git diff --check` passes. Firmware build unavailable locally because `arm-none-eabi-gcc` is missing; hosted all-project CI will verify. No release or hardware test.

## 2026-11-12 — First visible DSSMM browser skin pass

- Added “DSSMM” branding in the existing top tab strip, active-theme selected-row fill in the file browser, and a framed title/box artwork panel. The list data, up/down navigation, artwork lookup/cache and launch path remain the existing SuperFW implementation. No cross-project assets or hardware drivers were imported.
- Corrected `docs/PORTING-PLAN.md` to reflect the owner-reported Prime boot/theme smoke test and mark porting active. `scripts/test-theme-settings.sh`, `scripts/test-artwork.sh`, and `git diff --check` pass. No hardware screenshot or visual acceptance test yet; local full verifier unavailable without arm-none-eabi-gcc. Hosted matrix pending.
- Hosted completion: all four kernel jobs passed in run43317208545 on integrated sourcef9f3659, including the SuperFW three-board build/test suite. Only the docs-only result record follows that validated source. Prime visual feedback remains the next hardware-dependent checkpoint.

## 2026-11-13 — Default artwork, visible setting and SYSTEM paths

- The first Test 2 implementation gave Chis Title as the default. The cross-variant follow-up below extends DSSMM Light/Title defaults and legacy migration to SD and Lite too. All builds expose the saved List/Title/Box selector; versioned Test 2 explicit choices remain unchanged.
- New system save/state/log directories are `/SYSTEM/SAVEGAME/`, `/SYSTEM/SAVES/`, `/SYSTEM/SAVESTATE/` and `/SYSTEM/superfwlog.txt`. `/.superfw` remains unchanged. Root-level legacy save sets (including backups/temp) and savestates remain recognized and in use when there is no matching SYSTEM data; nothing is auto-deleted or moved. Added `docs/SYSTEM-LAYOUT.md` and refreshed setup guidance.
- The owner made consistent skinning across browser, settings, system/tools, game screens and in-game menus the top goal; the port plan and agent continuation now prioritize screen-by-screen skin work.
- Theme/settings/path regression tests, artwork tests and whitespace check pass. Local ARM builds unavailable because `arm-none-eabi-gcc` is missing; hosted validation pending. No release or hardware qualification.

- Follow-up to 2026-11-13 SYSTEM path work: hosted run43435382825 exposed a missing `config.h` include in `ingame_menu.c` (undefined SYSTEM_DIR and DISK_SAVESTATE_SLOTS). Added the include. Focused settings/artwork checks are rerun locally; hosted matrix must pass before dssmm integration. No hardware test or release.

- Owner requests a tidy workspace and cleanup of old/superseded builds going forward. After the final hosted matrix, removed ignored SuperFW compiler outputs and `superfw/build/` (including obsolete local Test1 staging builds); added a handoff cleanup rule. Published release assets/evidence and tracked source assets remain intact. The pre-existing untracked devcontainer lock remains untouched.

## 2026-11-13 — SuperFW Test 2 prerelease published

- Published `superfw-test2` as a prerelease with all three board images. Release title: “DSSMM SuperFW — Test 2 (Hardware Validation Pending).” Tag points to source `c748fea2286721da21aa3394cf8d4ac330b19c8a`; notes use the unified four-section order. Canonical notes and publication evidence are in `docs/releases/superfw-test2.md` and `docs/releases/superfw-test2-publication.json`.
- Local settings/artwork scripts passed; SD, Lite and Chis builds passed sequentially. Validated each board size limit, GBA header checksum/size field and full-image SHA seal. Downloaded all three public assets and confirmed byte equality, metadata and note body. Hosted project matrix passed on all five branches. Local full upstream host suite was unavailable because `lcov` is missing; hosted upstream host suite passed.
- No physical validation of Test 2 changes. Prior Prime v1.0 boot/theme smoke test remains limited to Test 1. DE `v8.4.0` remains latest stable; Test 1 unchanged. Removed generated binaries and superseded local build staging after verification. Next: receive Prime artwork/settings/save-path feedback and continue whole-interface skinning.

## 2026-11-13 — Synchronize DSSMM defaults across SD, Lite and Chis

The shared browser skin, artwork reader/panel, theme/accent controls and SYSTEM storage code were already compiled for all boards, but only Chis defaulted to DSSMM Light and Title artwork. Changed SD and Lite to the same defaults. Bumped saved UI settings to version 3 and migrate only pre-Test-2 unversioned/v1 native defaults; explicit choices recorded by Test 2 (v2) remain intact. Chis-only NOR and other real hardware differences remain separate.

Expanded theme/settings host checks to build with SD, Lite and Chis flags. `bash scripts/test-theme-settings.sh`, `bash scripts/test-artwork.sh`, all three sequential local board builds, and `git diff --check` pass. The full local `scripts/verify.sh` needs lcov, which is unavailable; hosted board matrix and upstream host suite are next. No new release or hardware qualification in this change. See `docs/PORTING-PLAN.md` cross-variant checkpoint and the prior Test 2 report for release history.

## 2026-11-13 — Three-variant DSSMM defaults and parity work

Aligned the shared UI defaults on SD, Lite and Chis: DSSMM Light plus Title artwork. Version-3 settings migrate only unversioned/version-1 native defaults; explicit version-2 Test 2 theme and artwork selections remain unchanged. Browser styling, settings controls, artwork lookup and SYSTEM paths were already shared across variants. Chis NOR remains Chis-specific. Expanded settings test builds to all board flags; settings/migration, artwork suites and all three sequential board builds pass locally. Release note draft: `docs/releases/superfw-test3.md`. Rebuild from the integrated commit and collect hosted matrix before publishing. No physical behavior is established.

## 2026-11-13 — Synchronized defaults published as Test 3

- Ported DSSMM Light and Title artwork defaults to SD and Lite, matching Chis. Version 3 settings migrate pre-Test-2 unversioned/v1 native defaults; saved version-2 Test 2 preferences are preserved. Browser skin, artwork controls and SYSTEM paths remain common; Chis NOR stays variant-specific.
- Published `superfw-test3` with all three images at source `ad3cdd53ec0eaa5239bf113b858e7b86b75838b9`, DSSMM Build 524. Five branch CI checks passed; local SD/Lite/Chis settings+migration tests and artwork tests passed, as did all three sequential builds and package checks. Public downloads byte-match builds and GitHub digests. CI host suite passed; local full `verify.sh` could not run because lcov is absent.
- Release is a prerelease with no Test 3 hardware validation. Previous Prime Test 1 boot/theme smoke is not extended; SD/Lite remain physically untested. Test 2 assets and prior releases remain. DE `v8.4.0` stays latest stable. Publication evidence: `docs/releases/superfw-test3-publication.json`.
- Removed generated local binaries/staging after byte verification. Root user overview now points to Test 3. Next: owner checks each available variant and report observations; keep styling shared where hardware permits.


## 2026-11-13 — Restore GBA artwork matching and simplify menu layout

Implemented artwork lookup for the reported letter-folder structure. Title mode now tries the CUSTOM ROM-filename BMP, the mirrored ROM-filename BMP, then the GBA header title under CUSTOM and the mirrored folder; Box mode tries CUSTOM and mirrored ROM filename only. Header reads use the 12-byte GBA title at 0xA0, reject unsafe names, and require no scans. Added adapter tests for CUSTOM priority, filename matching, header-title fallback, Box CUSTOM, exact reads and failures. Updated end-user artwork setup and prototype docs. Removed the cut-off artwork frame and top-bar DSSMM lettering. Left-aligned settings values, save/file-manager options and in-game savestate actions. Font glyph height is fixed at 16px; no current size control is available.

`bash scripts/test-artwork.sh`, `bash scripts/test-theme-settings.sh` and `git diff --check` pass. SD (499,200 B), Lite (466,944 B) and Chis (1,671,680 B) builds pass; each is within board size, with valid GBA checksum/size and firmware seal. These pre-commit images report build 527 and must be rebuilt from integrated source before release. `make verify-superfw` cannot run the coverage-enabled host suite locally because lcov is absent; hosted CI required. No new physical hardware validation. Next commit on `superfw`, normal merge to dssmm, sync all branches, collect hosted test results, build release assets from integration commit and publish as hardware-validation-pending prerelease. Owner asks pruning previous prerelease releases for SuperFW, BLUE and Original OMEGA to retain one newest release per lane; retain their tags, preserve DE stable.


## 2026-11-13 — Correct EZ-FLASH header artwork matching

Owner clarified that artwork fails generally and that Title assets are in `/SYSTEM/IMGS`. Reviewed upstream `ez-flash/omega-kernel/source/ezkernel.c`: `Load_Thumbnail` reads the four-byte GBA game code at 0xAC and opens `/IMGS/<code0>/<code1>/<code>.bmp`. Our earlier fallback incorrectly used the 12-byte game title. Added bounded/alphanumeric game-code path lookup under `/SYSTEM/IMGS` after Title CUSTOM and mirrored filename candidates; this supports native paths such as `/SYSTEM/IMGS/B/P/BPRE.bmp`. Box continues to use DSSCC filename matching in IMGS2, with CUSTOM first. Existing DSSCC DST1 transparency support is retained; the sanitizer suite already checks marked transparency and opaque black. Added regression fixtures for ROM filename priority, Box CUSTOM and code-derived header path.

Next run artwork/settings suites, rebuild all three boards, verify headers/size/seals and hosted CI before publishing. Local `lcov` is unavailable for coverage-enabled upstream host verification. No physical rendering has been confirmed.

## 2026-11-13 — Validate header-code correction

`bash scripts/test-artwork.sh` and `bash scripts/test-theme-settings.sh` pass after adding game-code matching. The artwork suite still verifies DSSCC DST1 transparency (marked zero transparent; unmarked black opaque), exact FatFs reads, CUSTOM/mirrored filename order, Box CUSTOM, and original EZ code path `/SYSTEM/IMGS/B/P/BPRE.bmp`. `git diff --check` passes. Local `make verify-superfw` reaches the coverage-enabled script but cannot run the upstream host suite because `lcov` is absent. The previous Test 3 image does not contain this correction; release remains pending complete hosted CI and final three-board builds.


## 2026-11-14 — Test 4 publication and release retention

Published `superfw-test4` at integrated source `19429735b3b55f7c57dd3921f0aad61601521e06` (DSSMM Build 531) with Chis, SD and Lite `.fw` assets. Release notes follow New features, New fixes, Summary of past features, Summary of past fixes. All five lane CI checks passed. Local settings/artwork tests and sequential board builds passed; size/header/seal checks passed; downloads match local bytes and GitHub digests. The full local verify script is blocked only by missing `lcov`; hosted complete host suite passed. Test 4 physical artwork and interface behavior remain unvalidated.

After publication, removed release pages/assets for superseded prereleases: SuperFW Test 1–3, BLUE Test 1–3 and Original OMEGA Test 1–3. Retained git tags and archived exact prior bodies/assets in root `docs/history/releases/prerelease-retention-2026-11-14.json`. Latest SuperFW/BLUE/OMEGA prereleases are Test 4; DE v8.4.0 latest stable and all DE historical releases remain. Public notes omit build headings, checksums, source archive notices and developer cautions. Root README and shared integration logs must be updated through dssmm, then sync all lanes and clean local generated artifacts.

## 2026-11-14 — Title artwork fallback, Info identity and parity continuation

Owner's Test 4 observation: Box art displays slowly on the SuperChis Prime; original ROM-header Title art still does not show. Updated the artwork decoder to read two image rows per FatFS call (half as many callbacks), support bottom-up and top-down 16-bit BMPs, and tolerate zero BMP file/image sizes when actual data is present. Added flat `/SYSTEM/IMGS/<CODE>.bmp` after the native nested EZ lookup. Tests cover both layouts, BMP orientations/fields, fewer reader calls, DST1, read errors and output bounds. `bash scripts/test-artwork.sh`, `bash scripts/test-theme-settings.sh`, and `git diff --check` pass. Physical title-art resolution remains unconfirmed.

Info now shows the 8-digit upstream hash immediately after SuperFW version, with DSSMM Build on a separate line. Board builds all passed sequentially. Installed `lcov` and `libssl-dev`; standalone `make -C superfw/tests all` passed with 90.8% line coverage. A full `make verify-superfw` completed all three builds but was interrupted at an intermittent upstream ASAN `DEADLYSIGNAL` loop; a focused full host suite passed separately. Do not claim one clean invocation of the integrated verifier.

Owner asks for autonomous menu porting to EZ-FLASH DSSMM visual/interaction parity, especially fluid carousels, and asks about emulator-based testing. Added the continuation plan in `docs/PORTING-PLAN.md`; inspected reference carousel implementation and confirmed it has substantial cache/scaling/motion logic. Debian mGBA 0.10.1 installed; hardware protocol testing remains physical-only. Next: build a small renderer seam and reference-matched list-browser surface before carousel motion; maintain separate SD/Lite/Chis validation and never share drivers across hardware families.

## 2026-11-14 — First browser carousel renderer slice

Added `browser_view` preference to UI settings v4 with List default and Horizontal/Vertical choices. Navigation changes one item per button in carousel modes. `browser_carousel_layout()` and fixed-point motion functions are shared by the firmware and a small GBA preview ROM, with host coverage for selection transitions, edge counts, bounds and centering. The kernel draws three adjacent cards and reuses its current artwork preview at rest. This is only a schematic first slice; icons, text, art scaling/cache, other menu surfaces and fluid DSSMM visual parity remain to implement.

Built/ran the preview ROM in mGBA 0.10.1 without a BIOS and captured F12 screenshots. Full SuperFW firmware still cannot get past SD/Chis/Lite card initialization in mGBA, so mGBA only validates the isolated geometry/render-preview path, not hardware I/O or true card timing. The mGBA harness code is at `tests/browser-preview-rom.c`; build with `scripts/build-browser-preview.sh`.

Host motion, artwork and theme/settings suites pass. SD/Lite/Chis builds pass sequentially after linking `browser_carousel.c`. Full upstream coverage suite previously passed separately after installing `lcov`/`libssl-dev`; the combined verifier had an intermittent ASAN hang. Re-run on integrated checkpoint and require hosted CI. The current Info hash UI and ROM-header flat path fallback are not hardware confirmed. Next: review and improve carousel visuals, reduce Box-art delay further, and continue DSSMM styling across browser/settings/tools/info/game and in-game surfaces.

## 2026-11-14 — Reduce artwork SD transaction count

Replaced per-scanline FatFS access with one read of the full bounded 16-bit BMP into a 4-byte-aligned 19,254-byte staging buffer in external SDRAM; decoder performs its two-row chunks from RAM. This reduces file read calls from about 21 to one per artwork candidate, likely addressing the reported Box slowness. Added a short-buffer regression check. Focused tests pass. Need all three final builds and hosted CI; no physical speed/artwork confirmation.

## 2026-11-14 — Match carousel viewports to EZ-FLASH DSSMM

Inspected the current EZ-FLASH horizontal/vertical layout constants and aligned the SuperFW selected and adjacent preview sizes/positions: horizontal 120×80 plus 60×40 neighbors; vertical 84×56 plus 48×32 neighbors. Implemented bounded nearest-neighbor `artwork_blit_scaled()` for vertical previews, including DST1 transparency and even-width VRAM writes. Removed card outlines, fixed right viewport clipping, and positioned selected filename beneath horizontal art or beside vertical art. No driver, card protocol, or launch action changes.

Validation: artwork, settings, motion, and diff checks pass; preview ROM compiles; clean serial SD/Lite/Chis builds pass after the DMA path and settled-motion read gate. Opaque unscaled framebuffer rows use DMA3; marked transparency and vertical scaling use software. Chis EWRAM is at about 93.0% linked region with existing unused-variable warning; linker regions fit. Local upstream host suite repeated AddressSanitizer `DEADLYSIGNAL` at `cimpl_test` and was stopped; no full local suite pass claimed for this run. Earlier mGBA schematic execution exists, but the fresh GUI screenshot attempt hung and was stopped. No hardware, artwork speed, or parity validation.

Next: cache neighboring artwork so movement avoids synchronous SD reads, then iterate browser background/selected-title styling and port common menu treatment to settings/system surfaces. Keep the temporary scratch file until session end.

## 2026-11-14 — First successful mGBA browser artwork frame

Packaged Debian mGBA 0.10.1 runs as `/usr/games/mgba` (not plain `mgba` on the default PATH). Built the synthetic preview ROM and executed it under Xvfb with dummy audio. A 240×160 screenshot visibly contains the horizontal carousel and generated Title art selected by the fake GBA header code; shared GBA decode/blit code executed. This validates only the isolated preview harness. It does not validate SD/FatFS lookup, the real kernel browser, SuperCard interfaces or hardware timing. Fake ROMs are intentionally non-launchable; no BIOS was needed.

Box artwork and emulator key-driven view changes remain unresolved: one keyboard experiment yielded an unusable frame and is not counted. Earlier mGBA 0.10.5 AppImage attempts showed a blank screen; record exact binary/version because packaged 0.10.1 now renders Title mode. Root standard-container changes/logs are committed separately on dssmm (`9c577cea`, `874a2ee`) and should be merged normally after this lane's source checkpoint. Keep screenshots/fixtures ignored and BIOS out of tracked/public content.

Added and passed `bash scripts/test-browser-preview-mgba.sh`: packaged mGBA 0.10.1 renders a 240×160 frame with the known Title-art marker. The script waits 5 seconds for fresh Xvfb startup, saves F12, and checks screenshot pixels; 3 seconds was insufficient in one run. This is only an isolated renderer smoke test. Box art, key input, filesystem lookup and full firmware behavior remain open.

## 2026-11-14 — Animate production carousel artwork

Selected artwork now follows the layout's moving/scaling rectangle. Cache identity must match ROM path and artwork mode, and failed/missing blits fall back to the focused icon. This removes stale artwork during deferred reads and fixed-position art during transitions. Artwork and motion/fixture host suites and diff checks pass. Three serial board builds are running in session 41036; verify its result and `/tmp/dssmm-carousel-*.log` before recording success. The first launch lacked devkitARM on PATH and was corrected. Full visual parity remains incomplete.

## 2026-11-14 — Reject oversized scaled-artwork rectangles

Added explicit screen dimension limits before unsigned clipping arithmetic in `artwork_blit_scaled`; oversized dimensions previously could underflow the bounds subtraction. New regression cases cover width 242, height 161 and near-UINT32_MAX width. Artwork/adapter tests and diff checks pass. Revalidated build session 41036: SD/Lite succeeded and Chis remains live. These builds overlap the latest guard edit and must be rerun on a settled source checkpoint before release evidence is claimed.

## 2026-11-14 — Double-buffered preview and meaningful BMP assertion

Three-board session 41036 ended successfully; final latest-source board rerun still required. Preview now renders hidden Mode 4 page then flips at VBlank. Added deterministic PREVIEW_VERTICAL/PREVIEW_BOX build options. Replaced incorrect orange-marker assertion (it matched a UI card fill) with counts excluding all four UI colors. mGBA 0.10.1 checks pass for horizontal Title (13,154 decoded-BMP pixels) and vertical Box (4,209). Earlier marker-based passes alone were weak evidence and are superseded. Actual SD/header lookup and hardware Title-art result remain open.

## 2026-11-14 — Four-mode mGBA coverage and RGB555 unused bit

Added PREVIEW_MATRIX=1 smoke mode and confirmed all four initial view/art combinations render decoded BMP colors (13154/8593/6689/4209 pixels). Session 30060 ended exit 0. Fixed decoder rejection of unused RGB555 bit15; it is now masked before palette conversion/transparency. New opaque and DST1 regressions pass with the artwork adapter suite. The edit overlapped matrix compilation; final latest-source board/emulator checks remain required. Physical Title art and full menu parity remain unverified.

## 2026-11-14 — Cache adjacent carousel artwork

Production carousel now uses three SDRAM artwork slots keyed by path/mode, including cached misses. Loads one missing selected/previous/next image per settled frame and draws matching neighbor art during transitions without SD reads. Existing invalidation points clear slots except ordinary carousel movement. Full-size selected art keeps DMA path. Build session 38222 is live; inspect `/tmp/dssmm-cache-*.log`. A tiny DMA-path edit overlapped startup, so final settled-source builds remain necessary. Host checks session 36744 must be inspected. Goal remains active with remaining surfaces and physical title-art confirmation outstanding.

## 2026-11-14 — Repair cache lifetime and align game options

Fixed unconditional cache clearing introduced in the prior edit; key-action invalidation is conditional and now clears metadata flags through a helper rather than zeroing decoded image buffers. Artwork/motion/fixture and three-variant settings suites pass. Left-aligned game patch/loading headings and bounded label/value columns to prevent overlap. Visual popup validation remains pending. Build session 38222 is live but overlaps newer source edits, so final board validation still required.

## 2026-11-14 — Official thumbnail pack identified

Owner supplied the official IMGS.zip URL (https://www.ezflash.cn/zip/IMGS.zip). Curl and web access return Cloudflare HTTP 403; no pack validation occurred. Recorded URL and limitation for subsequent sessions. Actual pack paths/pixel format still need direct inspection when the archive is available.

## 2026-11-14 — Info/Tools layout and final-source build checkpoint

Session 38222 completed all three builds; newer menu/cache edits require fresh evidence. Info text now uses left-aligned bounded lines and Tools labels use available width. Artwork guide now describes neighbor cache instead of claiming it is unsupported. Production source frozen for serial board session 42756 (`/tmp/dssmm-checkpoint-*.log`); focused host suites in session 33143. Inspect terminal results before success claims. Remaining parity/hardware requirements are still open.

## 2026-11-14 — Verify actual carousel cache orchestration

Added/passed host regression compiling extracted production invalidation/render functions with actual layout and mocked storage/draw calls. Asserts single-load settled frames, neighbor/miss caching, no motion reads, mode isolation and flag-only invalidation. Included in scripts/verify.sh. Board session 42756 is still live; production source unchanged during this turn. Full menus/card title-art remain unqualified.

## 2026-11-14 — Cache directory boundaries and visual ordering

Expanded production-cache regression for path changes during motion and directory entries; passes. Visual review of vertical Title screenshot identified next art covering selected art due previous/selected/next painter order. Defer production fix until frozen build 42756 is terminal (SD passed, Lite live), then draw focused card last and test ordering. Pixel-presence smoke checks alone do not prove visual parity.

## 2026-11-14 — Frozen-source builds pass all boards

Build session 42756 ended exit 0 with SD/Lite/Chis firmware. Production source stayed unchanged throughout this run. Focused host suites pass. Preserve this checkpoint, then fix confirmed vertical painter ordering. Full verifier/hosted CI, visual parity and hardware Title art remain outstanding.

## 2026-11-14 — Checkpoint and correct focused-card layering

Committed validated carousel/cache milestone as 944ec48. Fixed focused-last painter ordering in the shared layout, added explicit geometry assertions, and passed motion/fixture/cache tests. mGBA vertical Title screenshot passed (6700 BMP pixels) and was visually inspected. All-board evidence currently applies to 944ec48; painter follow-up needs final build/CI. Parity and actual Title-art card behavior remain incomplete.

## 2026-11-14 — Synchronize shared mGBA setup and scroll carousel names

Merged origin/dssmm normally into the lane, carrying root mGBA environment/documentation commits. Selected carousel names now left-align within bounded text regions and rotate to reveal long filenames. Cache and motion/fixture host tests pass. Latest source still needs visual/menu/board validation; CI 43617571436 was live at inspection. No release or parity claim.

## 2026-11-14 — mGBA preview uses production filename text

Linked real font renderer and extracted production scrolling helper into preview build, including left-aligned selected names. Vertical Title mGBA check passes and screenshot visually inspected. Documented preview-only warning suppression for preexisting Hangul declaration style. CI 43617571436 passed; newer 43617663224 remains live. Complete production menu/hardware behavior remains unverified.

## 2026-11-14 — Transition preview and layout bounds sweep

Added fixed-offset and automatic-retarget modes to the mGBA harness; filename follows visual focus. Vertical midpoint screenshot inspected and smoke passed; automatic-motion smoke passed. Geometry sweep covers both orientations and all selections/counts1..5 over -768..768 offsets, passes bounds and focused-last invariants. CI ae0ca6f passed; newer preview CI live. Whole-menu parity and hardware/title-art remain unverified.

## 2026-11-14 — Actual Tools text rendered in emulator

Added PREVIEW_SCREEN=1 harness using production Tools function/messages/font helpers. mGBA text check passes; screenshot reviewed with all five common rows fitting. Selection OAM and actions are excluded. Production firmware unchanged. e44bde4 CI passed; newer run live at inspection. Continue menu parity and hardware/title-art work.
