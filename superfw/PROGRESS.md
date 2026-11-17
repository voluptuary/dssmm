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

## 2026-11-14 — Track hardware candidate and fix edge arithmetic

Recorded owner Build534/hash126ae458 findings in BUILD-TRACKING.md. Commit lacks carousel, proving dirty-source candidate; future builds get board identity manifests including project fingerprint. Fixed unsigned navigation subtraction; actual production-navigation regression fails before and passes after. Missing art/sluggishness remain open. Next board/CI validation and measured renderer/storage investigation.

## 2026-11-14 — Scaler arithmetic optimization

Precomputed horizontal nearest-neighbor sampling positions instead of dividing twice per pixel pair on every row.120x80 division count falls9680→200; no measured hardware speed claim. Pixel-exact regression sweep9 widths×6 heights×transparent/opaque passes plus adapter/navigation tests. Diagnostic dirty-source board build session3360 live; preserve manifests and freeze source until completion. Commit and clean-build before distributing a candidate. Actual artwork/card-performance findings unresolved.

## 2026-11-14 — Measure preview drawing cost

Added optional cascaded GBA timer readout. Vertical Title mGBA screenshot passed and reported1364130CPUcycles (~4.86frames). Optimized scaler still leaves full redraw too expensive for fluid movement; next cache scaled images/avoid unchanged redraw. Fixture memory differs from real cartridge SDRAM. Edge-fix CI passed; diagnostic board3360 live. No hardware-performance claim.

## 2026-11-14 — Animation On/Off setting

Owner requested immediate versus animated carousel changes. Added persisted format5 carousel_animation, default On, UI toggle, motion reset and gating for both orientations. Settings tests pass for all boards plus cache regression. Prior diagnostic build3360 completed successfully but predates toggle. New source requires clean numbered builds and CI. No physical fix/performance claim.

## 2026-11-14 — Native-size fast carousel path

Animation Off now draws selected artwork at native dimensions, uses neighbor icons, and skips neighbor preloads and all scaling. Production renderer regression verifies valid art, zero scaler calls in both orientations and selected-only load. Passes. Build92051 predates this change; latest board/CI/hardware validation required. Goal remains active.

## 2026-11-14 — Validate no-scaling Off mode

Actual-renderer regression now covers Title/Box and horizontal/vertical native blits with bounds assertions, no scaler calls and no neighbor preload. Navigation/cache/settings checks pass. Prior547 build sequence finished but Lite/Chis recorded dirty source; preserve as diagnostic only. Commit before fresh uniform candidate builds; no release or hardware-speed claim.

## 2026-11-14 — Native-size Title/Box neighbor placeholders

Owner requested generic artwork icons in fast mode. Implemented original16x16 landscape screen/upright box patterns, using theme palette and halfword framebuffer writes. Actual helper and cache tests pass distinct shapes/padding/bounds and no-scaling/no-neighbor-read invariants. Build548 predates icon edit; latest source still needs board/visual validation. Goal active.

## 2026-11-14 — Visually validate fast Box placeholders

Preview now calls actual placeholder helper and supports AnimationOff layout. Vertical Box screenshot passed and visually shows upright neighbors with native selected image. Recorded fixture timing without claiming hardware speed. Build52898 terminal success but source overlapped latest edits, so548 set remains diagnostic. Next clean committed candidate builds/CI and continued rendering optimization.

## 2026-11-14 — Reduce transparent native-blit reads

Opaque pixel pairs in marked art now avoid background reads. Native pixel-reference and artwork/adapter suites pass. Same vertical Box Off fixture985694cycles versus1051587 before (~6.3% reduction), screenshot unchanged. Still costly; hardware response/artwork unresolved. Commit and hosted board validation next.

## 2026-11-14 — Skip unchanged artwork redraw in fast mode

Added per-Mode4-page identity tracking for AnimationOff; stable pages retain body artwork and refresh only text/footer. Menu/modal/artwork invalidation resets both pages. Extracted renderer test verifies no art blits on reuse; passes. Page-lifetime integration, board and physical/runtime performance validation remain required.

## 2026-11-14 — Verify page reuse lifetime

Production-policy extraction regression passes both page identities, selection/view/mode changes and all modal/tab/animation invalidation paths. Artwork reset test confirms both pages invalidated. Added to verifier; no firmware change this turn. Full runtime/hardware parity still unproven; newest CI live at inspection.

## 2026-11-14 — Candidate identity/integrity gate

Added three-board identity/firmware validation tool; current mixed diagnostic set correctly rejected as Lite dirty. Documents clean manifests, same source/number/fingerprint, hashes/checksum/size/seals. Does not claim latest boards/hardware pass. Continue clean candidate and runtime/menu parity work.

## 2026-11-14 — Clean Build553 set and honest timing baseline

All three clean553 builds completed; same source/fingerprint and full integrity checks pass. Persisted manifests in docs/build-553-validation.json; CI43619877608 passed. Corrected preview CPU fills to DMA and generated missing messages dependency locally. Scratch clean preview passed; DMA BoxOff433206cycles supersedes985694CPU-fill figure as closer preview baseline. No physical/full parity claim.

## 2026-11-14 — Local Chis553 hardware candidate supplied

Placed validated Chis553 image, manifest and guide in root hardware-test as requested, locally excluded from Git. Confirmed firmware hash matches clean553 evidence. Shared direct path with owner and resumed notes/porting. Test5 note draft follows four ordered cumulative sections and explicitly retains534 failures/unverified fixes/parity limits. No release publication or hardware result yet.

## 2026-11-14 — UI-settings screen in emulator

Actual settings renderer/helpers now previewable with PREVIEW_SCREEN=2. mGBA check passes and initial rows visually inspected. Fixed harness-only footer interference. Fixture state/OAM/input limits documented. Firmware and hardware-test553 unchanged; goal active.

## 2026-11-14 — Validate settings lower page

PREVIEW_OPTION uses actual settings scroll policy. Save9 lower-row screenshot passes and visibly fits all rows/button. No production firmware change; mGBA input/persistence/OAM not qualified. Continue theme/menu parity while owner tests553.

## 2026-11-14 — Production Light palette checked

Preview extracts actual theme data/type; Light6 settings Save screenshot passes and visually reviewed. Foreground-presence smoke includes controls, documented. Production firmware unchanged; continue skinning/interactive/hardware validation.

## 2026-11-14 — Edge crash hardware-confirmed fixed; artwork fails

Owner Chis553 reports edge crash resolved but art still absent. Reverted Off neighbor artwork placeholders to native system icons as requested; helper removed, preview/cache tests adjusted. Cache/navigation regressions pass. Continue real SD→SDRAM artwork-path investigation, especially byte-write constraints; themed selection-row work also awaiting preview/build validation. Full goal incomplete.

## 2026-11-15 — Reproduce carousel-only hardware memory defect

Owner confirms List Box art works. Carousel metadata was in cartridge SDRAM despite bool/string byte writes; List metadata is internal. Fixed placement, reverted custom neighbor placeholders, deferred unrelated skin changes. ARM Thumb cache regression rejects Build553's byte store at0x080026a0 and passes corrected renderer; real ELF places816-byte metadata in IWRAM. Added mandatory linked-address check per board, optional Unicorn diagnostic and detailed evidence/limits. Finish numbered board validation and Chis hardware-test delivery, then pause per owner request; no physical artwork fix claimed yet.

## 2026-11-15 — Chis561 delivered; owner authorizes continued port

Clean SD/Lite/Chis Build561 at76b7a493 share fingerprint and pass image identity/seal checks; each links816-byte metadata to IWRAM0x03000e74. Focused suites, ARM bus regression and all four AnimationOff mGBA Title/Box/orientation previews pass. Hosted full verifier43663344006 passed. Local upstream rerun hung with ASAN DEADLYSIGNAL in crc_test and was terminated; no local full-pass claim. Evidence: docs/build-561-validation.json.

Copied Chis561 to root hardware-test/superfw-chis-build561.fw, SHA ffaf6863e320ec5aeb9c6521a7510167e58403e67508c17e9291f987cf8940ef, with manifest/guide. Removed our superseded553 local image/manifest; historical tracked evidence remains. Owner has switched models and explicitly revoked the requested pause: continue porting after delivery. Physical carousel result remains pending; retain561 unchanged while later menu work proceeds. No public release this turn.

## 2026-11-15 — Solid themed menu selection rows

Continued port after owner revoked pause. UI-settings/Tools for DSSMM Light/Dark use accent fills behind text and avoid native tiled overlays; upstream themes unchanged. mGBA Light/Dark settings/lower row and last Tools action visually fit, cache/page regressions pass, Chis selection-code link fits. Removed duplicate knob arrows seen in preview. Follow-up hosted suite needed; current test image remains561 with metadata fix. Physical artwork and full menu parity remain pending.

## 2026-11-15 — Follow-up menu validation complete

Source fd719d515a65ec7249bb829f2d306f0917e35df8 passed hosted full SuperFW verification43663757321 (three board builds, linked metadata gate and host suites). Final Dark lower-settings screenshot verifies duplicate arrows removed; earlier Light/settings and Tools screenshots inspected. Cleaned generated compiler/host-test/build folders after retaining validation evidence; kept root hardware-test Chis561 and its matching manifest/guide intact. User devcontainer lock, session scratch and private BIOS preserved.

Next: await owner561 physical carousel result, then continue settings/game/in-game menu parity and real interaction/performance coverage. Do not treat isolated mGBA previews as card emulation. The mandatory artwork fix is published to the superfw source branch, not a new public release. Current development source is newer than the intentionally frozen561 test image.

## 2026-11-15 — Physical filename artwork confirmed

Owner561 confirms filename art now works; header Title unresolved. Slow On/Off and alignment feedback recorded. Started native-size cropped neighbor rendering and late-cache page invalidation; decoder suite passes, cache regression being adapted to neighbor loading. No new candidate or performance claim. Continue artwork format investigation and production performance/parity validation.

## 2026-11-15 — Native cropped neighbors and settled-frame reuse

Implemented native previous/current/next Off layout with edge/body clipping and selected-last layering, using production cropped blitter in preview. Opaque cropped rows use DMA. Both animation modes reuse settled pages, avoiding idle repeated scaling; motion/modal paths invalidate. New slot loads invalidate both pages so late neighbors become visible. RGB555 quantization uses exact32-entry lookup instead of three divisions per pixel. Pixel-reference cropping tests cover left/right/top/bottom and transparency/canaries; artwork/cache/page and ARM bus tests pass. Four Off mGBA screenshots pass; physical timing remains unconfirmed and synchronous SD reads can still stall.

Compared DE game-code reader: fixed54+19200-byte direct-pixel record, unlike our strict BMP metadata parser. Added exact-length BM-only legacy interpretation after strict decode fails, ONLY for game-code Title fallback; metadata normalized in internal RAM. Adapter regression covers inconsistent desktop metadata and strict filename rejection. Official IMGS archive still403; this is a compatibility hypothesis, not pack/hardware confirmation. Next clean numbered all-board builds/CI and fresh Chis candidate, then continue parity/performance work without pausing. User explicitly requests sustained autonomous continuation.

## 2026-11-15 — Build565 ready for card retest

Clean uniform SD/Lite/Chis565 builds, integrity gate and hosted43665489569 pass. Chis image delivered in hardware-test with manifest/guide, replacing superseded561 local artifacts. Owner561 filename success retained;565 native neighbors/performance/legacy Title need physical result. Continue alignment while keeping565 frozen.

## 2026-11-15 — Animated DE geometry preserved with clipping

Corrected horizontal interpolation to move image centers (DE left/right-5/185) and vertical next Y124. Removed fit-to-body geometry shifts; actual clipped scaler keeps source sampling anchored while protecting UI rows. Missing-image icons clamp independently, selection paints last. Added pixel-reference/canary regression for clipped scaling; motion/cache/page/navigation/ARM bus checks pass. All four settled animated mGBA views pass/visually reviewed; Chis source link fits (EWRAM94.13%,IWRAM39.28%). Mid-transition preview pending. No new distributed image; hardware-test565 frozen. Full parity/performance and physical legacy Title result remain open; continue autonomously.

## 2026-11-15 — Alignments validated, common selection treatment extended

All-board/full CI43665956534 passes DE geometry/clipped scaler; vertical midpoint preview passes. Main settings adopts existing accent row helper for Light/Dark, Chis link fits and shared UI preview passes. Main settings screen visual/input remains unqualified. Draft release notes corrected for current behavior/physical evidence. Integration next; hardware-test565 untouched.

## 2026-11-15 — Main settings emulator coverage and integration checkpoint

391d7b7 passed hosted43666119527 all-board/full verifier. Added actual main-settings renderer/enum extraction to preview (Screen3), with clearly synthetic settings/date fixture and optional Chis NOR rows. Visually inspected Light Chis Fast-ROM, Dark SD lower save-policy and Save screens; smoke checks pass. Long values remain bounded/truncated; input/persistence are not emulated. Firmware inputs unchanged by this harness slice. Next normal merge into dssmm and combined CI; hardware-test565 remains fixed for performance/header-art result. Continue autonomous parity work; do not claim full parity or physical565 confirmation.

## 2026-11-15 — Selected image hot cache; combined CI passed

Integrationc7872aa passed all four hosted kernel jobs43666315152. All hardware lanes synchronized normally through dssmm: DE2da7584,OMEGA2c2d5cc,BLUE4b9255c; no force/squash or driver changes. Current new SuperFW follow-up adds selected-image DMA copy into9,600-byte EWRAM NOLOAD cache, so repeated sampling avoids cartridge reads. Three SDRAM backing slots remain; identity invalidates on load/path reset and tracks selection. Cache regression verifies no repeated copies when stable; ARM bus checks pass. Actual Chis link places hot pixels in EWRAM and fits97.95% (5268bytes spare), checked by mandatory ELF gate. Hardware speed remains unmeasured, and565 test image is unchanged. Need clean all-board builds/hosted gate of this follow-up before distributing another candidate; continue autonomous work.

## 2026-11-15 — Clean571 all-board hot-cache candidate delivered

All SD/Lite/Chis571 builds at3d0b1a80 are clean, identical source fingerprint, linked metadata/hot-buffer gates and image identities/seals pass. Hosted43666618620 full verifier passes. Evidence docs/build-571-validation.json. Current root hardware-test image is Chis571 (hash3d0b1a80), replacing our565 local image/manifest after retaining evidence. Includes cropped native neighbors, legacy code-art fallback, DE-position clipping and selected EWRAM hot cache. Physical performance/Title result pending. Continue autonomous info/game-menu parity; do not change this frozen candidate. No public release.

## 2026-11-15 — Game information/patch/loading presentation

ROM-info text is bounded/left-aligned with scrolling filename; game action button remains centered. Patch/loading Light/Dark rows use common accent fill; native OAM overlays retained for native themes and NOR driver/action paths unchanged. Headers now leave room for navigation arrows. Compact English Database/Engine/None/Skip BIOS labels and two-digit-year date with wider RTC column fix observed truncation. Screens4/5/6 extract actual renderer functions into mGBA preview with synthetic field fixtures, borders and text navigation arrows; actual launch, storage, NOR and disk OAM are not emulated. Light Flash128K info, Dark long-name/unknown-save, patch In-game and loading RTC previews pass and were inspected. Fixed fresh-preview messages shadowed by stale src/messages_data.h via local wrapper copy. Chis link pending final header adjustment; final hosted all-board tests required. Frozen hardware-test571 remains current; no physical571/art/speed claim.

Final local game-screen result: Chis link passes with252072/257024EWRAM bytes (98.07%) and hot-image/metadata ELF gates pass. Framed patch preview visually confirms arrows/header separation; focused settings/cache/page/ARM bus checks pass. Source checkpoint next, then hosted all-board verification. Candidate571 remains unchanged.

## 2026-11-15 — In-game main/reset/save selection slice

Added solid selection rows to native in-game main/reset/save screens using existing accent palette color, across manual themes. Copied the existing OBJ accent into unused background palette20 (logo uses1..3; text16..19); no palette/header layout, action, save/reset, card protocol or restoration changes. These three screens suppress tiled OAM bar; other in-game screens retain it. Popup-active helper suppresses underlying highlight. Sanitized extracted production helper checks all Ypositions, exact pixels, canaries and popup suppression; added to verifier. Chis in-game payload link fits48904/61440 bytes and combined firmware252184/257024; no physical IGM qualification.

New previews7/8/9 extract actual in-game presentation only with namespaced translations and synthetic flags. Light main/RTC and Dark reset/save Back screenshots pass and were inspected; logo/OAM/actions/storage/register restoration excluded. Current hardware-test571 predates this UI slice and remains frozen. Next hosted full all-board verifier, then integrate latest through dssmm and synchronize lanes. Continue performance/parity; ROM-header art and571 speed still need owner hardware results.

## 2026-11-15 — Hot buffer follows visual transition focus

Refined selected EWRAM cache to follow item->focused, not the new logical selection during the first half of a slide. This keeps the largest visible image in GBA RAM while names/geometry transition; idle/Off focus remains the selected file. Production cache regression asserts the previous image remains hot at the midpoint; ARM bus regression passes. No hardware speed claim. Latest source needs hosted all-board verification before final integration;571 candidate remains fixed and predates this refinement.

## 2026-11-15 — In-game selection consistency and parity map

Extended the shared in-game solid row helper to RTC Update, cheat rows and state actions, keeping numeric arrows/slot icons and all action/storage/restoration code native. State make-persistent selection is normalized before highlighting, as before. Screen10 actual RTC renderer with fake date/speed passes and was inspected; full cheat/state render fixtures remain pending. Bounds/popup helper sanitizer passes. e4c486b hosted43667997240 passed hot visual-focus refinement plus prior IGM slice. Latest extended IGM source still needs hosted all-board verification. Added docs/UI-PARITY.md as the live surface/performance/evidence map; corrected primary reference to ezfomegade per owner. Continue autonomously, with571 hardware image frozen.

## 2026-11-15 — Full in-game renderer fixture coverage

5ae64a7 passed hosted43668232783 full all-board verification, including new selection sanitizer. Added production cheat/state render extraction with native icon assets and synthetic flags/titles; preserved signed slot counts after initial fixture compiler failures exposed incorrect unsigned mocks. Memory slot, persistent slot, make-persistent-normalization and disabled cheat row screenshots pass and were inspected. Palette128..143 loaded only in slot previews to avoid corrupting artwork colors elsewhere. All slot/save/cheat action and restoration paths remain excluded; physical gameplay/state compatibility not qualified. This follow-up changes tests/harness/docs only, not firmware. Integrate the validated firmware milestone normally and keep hardware-test571 frozen.

Final in-game fixture result: all memory/disk/copy/cheat screens pass mGBA0.10.1 smoke checks and visual inspection, using original renderers/icons with synthetic data only. Source firmware5ae64a7 remains the last tested all-board milestone; newer d80ccac+fixture edits are harness/docs only. Continue autonomous interaction coverage and normal integration;571 remains frozen.

## 2026-11-15 — Actual UI input replay checked in mGBA

Screen2 now routes GBA keys through actual extracted UI-settings key handler, with signed selector/offset like production, actual theme/accent resolver, and explicit no-op artwork/filter/false-save boundaries. Automated PNG marker asserts live Animation state. Right=>On, two Rights=>Off plus five Downs=>Accent, and four Ups=>top clamp then Right=>Dark pass; screenshots inspected. Repeated On check after make clean also passes without firmware-generated headers. Fixed production selection comparison with explicit int cast, matching actual selector type and avoiding preview -Wextra signedness warning; behavior unchanged. Native event handler is exercised, persistence/full browser filtering are not. Integration4ebcf3b hosted43668765783 passed all four kernel jobs; latest input-harness/cast checkpoint still needs hosted gate. Removed superseded board/compiler/test outputs after preserving571 evidence/hardware image; session BIOS/scratch/user lock preserved.

## 2026-11-15 — BMP staging tail bus fault reproduced and fixed

Clean uniform581 source814ee8e all3 packages and integrity gates passed (docs/build-581-validation.json); root all4 CI43669205497 passed and all5 trees matched65affc9c. Before distribution, audited linked memcpy+FatFS final-sector copy: STRB on final2BMP bytes. Optional actual-adapter ARM bus test rejects the pre-fix source at0x08004b34 and corrected code passes. Changed to word-aligned bulk request plus GBA-RAM tail copied with halfword stores; immediate tail read reuses cached sector. Compile-time FatFS cache alignment assertions guard source layout. Mandatory adapter test checks request alignment, misalignment rejection, tail/short-read/close faults. This is pixel-tail correctness, not confirmed cause of absent header Title. All-artwork sanitized suite passes; latest ARM replay pending final added fault tests.581 was not distributed;571 stays in hardware-test until replacement. Need new committed clean all3 package/hosted gate, then provide current image and continue. No pause or physical qualification claimed.

Final BMP-tail checks: sanitizer and ARM adapter replay pass normal/short/close/tail failures, all tail lengths0..3 with valid odd BMP offsets, odd-capacity padding rejection, and buffer misalignment rejection. Both ARM tools now reject any non-halfword/word or improperly aligned cartridge store. This closes the reproduced tail fault without reverting to per-row reads. Commit and clean-package replacement next; no further source edits during packaging.

## 2026-11-15 — Clean582 current hardware-test candidate delivered

All SD/Lite/Chis582 at842bad386c5c3f5616a4b01eaa4f5e3f8596c5c2 are clean, share fingerprint, and pass image identities/seals plus linked metadata/hot-image placement. Hosted43670448850 full verifier passed. Chis uses252376/257024EWRAM and12872/32768IWRAM; static layout does not qualify stack/timing. Evidence docs/build-582-validation.json. Copied Chis582 to root hardware-test/superfw-chis-build582.fw, SHA5f3e919b878a252056c4bc31d1db869868cfca39ec0f1ade558d4680876350de, with manifest/guide. Superseded our571 local file/manifest after retaining tracked evidence.581 was never distributed. All broader menu/input previews remain applicable because staging fix only changes storage adapter.

Owner has not yet reported582 Title/speed/IGM results. Filename art and edge fix remain physically confirmed only on earlier owner-tested builds. No public release. Next normal merge/sync of final staging fix/evidence, shared checks, collect combined CI, clean generated outputs while preserving hardware image and session BIOS/scratch/user lock. Then continue performance/parity from docs/UI-PARITY.md; do not claim full parity or pause for permission.

## 2026-11-15 — AGS lead verified and requested simplification

Build582 owner report recorded. Exact DE fix944ac13 accepts AGS planes0; not DST1. Broadened game-code reader to native bounded opaque records, preserving native marker checks; No-Intro paths tested unchanged. Removed Animation option/instant route, migrated settings6, retained standard transitions. Added selected±2 cache with no redraw for far prefetch; centered Info version/hash. All focused suites/ARM bus tests and four mGBA carousel views pass. Original pack inaccessible403, physical fix/speed unconfirmed. Latest changes need clean numbered all-board builds/CI before hardware-test replacement; no feedback/permission pause requested.

Current implementation checks pass for all four carousel previews, actual View input, legacy/native marker and No-Intro adapter fixtures, five-slot/far-prefetch cache, old-setting rejection and ARM bus constraints. Ready for clean numbered three-board packaging and hosted validation.

## 2026-11-15 — Build585 validated and delivered

Clean uniform SD/Lite/Chis585 source45e4dbbcc70fcb83d83d55b465e52118eb750ad0 all pass image identities/seals and memory gates; hosted43716700372 full verifier passes. Chis IWRAM13416/EWRAM251732; metadata1360bytes (5slots), focused cache9600EWRAM. Evidence docs/build-585-validation.json. Current root hardware-test/superfw-chis-build585.fw hash45e4dbbc SHA4f1ae0b0e2648b05511a41bfa1217ecc9dcda95fefe24502930405ade00028fa, matching manifest/guide; removed our superseded582 local artifacts after preserving evidence. Hardware compatibility/speed not claimed; original pack unavailable403, synthetic/native-source behavior verified. No public release.

Continue autonomously into directory-open cost: DE history shows header-art ROM reopen scans long-name directories repeatedly. Inspect native SuperFW FatFS R0.15 (LFN2/UTF8, exFAT1, lock/reentrant0) and design independently validated read-only indexed header open with full name/size checks and normal f_open fallback. Do not copy another card's driver. Browser has16384records; current SDRAM state15080036bytes,14.5MiB limit15204352, so a4-byte per-record index(+65536) fits,8bytes(+131072) does not. dir_next leaves dptr unchanged at EOC, so do NOT infer prior SFN offset by subtracting32 after f_readdir. Capture blk_ofs/LFN start (or current dptr for SFN) inside native readdir before advancing, and validate reconstructed name/size when opening. FAT/exFAT real-library RAM-disk tests are needed before retaining any change. Keep585 hardware image frozen through this next work.

## 2026-11-15 — Owner585 confirms artwork/cache; folder art and font goals

Owner reports585 header Title images now display and five cached images solve the reported neighbor problem. Record this as physical Chis585 evidence only. New focus: CUSTOM directory art (`Game Boy Advance.bmp` in both IMGS/CUSTOM and IMGS2/CUSTOM), and optional DE DSSMM font. Added docs/FOCUS-GOALS.md and current agent guidance. Defer indexed filesystem work; only its native source was inspected, no FF/library changes made. Begin folder-art loader/renderer support then font asset/metrics/provenance audit; preserve585 image until a validated replacement. Continue autonomously.

## 2026-11-15 — CUSTOM directory artwork implemented

Added dedicated validated directory-path/loader API: exact complete basename in mode-specific CUSTOM, preserving dots, rejecting parent/traversal names. Strict DSSCC decoding/DST1 and word-safe staging reused; no ROM header/mirrored fallback for directories. List+Art and both carousels dispatch folder entries through this loader, cache positive/negative results and draw valid art instead of folder placeholder. Missing/invalid art retains native icon. Sanitized path/format fixtures cover owner Game Boy Advance paths in Title/Box, dotted names, wrong dimensions, missing images and no ROM opens. Actual carousel dispatch/cache regression and both ARM bus tests pass. Updated end-user artwork guide (post585 development availability). Next optional DE font asset conversion/metrics/persistence/IGM propagation; no new hardware candidate yet,585 remains frozen.

## 2026-11-15 — Optional DSSMM system font

Implemented Native-default saved Font choice and actual DE printable ASCII
bitmap in native byte-safe renderer, with native Unicode fallback. Preserved
Apache notice and documented conversion/metrics in FONT-PORT.md. Settings and
pixel/scroll/clipping sanitizer pass; Chis link and mGBA UI fixture pass.
In-game reserved-word propagation retains existing payload layout. Clean all3
packaging and new hardware-test image follow; no physical font claims.

## 2026-11-15 — Build588 candidate

All3 clean588 packages pass identities/seals/memory gates; hosted43720266778
passes. Local full sanitizer suite passes with non-PIE flags after standard
ASAN startup failure; evidence build-588-validation.json. Delivered numbered
Chis588 in root hardware-test with CUSTOM directory art and optional actual DE
ASCII font. Updated guide/manifests and removed superseded own585 artifacts.
No public release or physical588 qualification. Normal integration e7759f3
pushed; finish evidence integration/sync and retain session BIOS/scratch.

## 2026-11-15 — SYSTEM-only resources and simpler settings

Moved configuration/cache/cheat/emulator/recent/backup/pending resource paths to
SYSTEM. Added resumable no-overwrite startup migration before pending SRAM;
conflict/errors stop boot. Removed directory choices; canonical SAVEGAME and
SAVESTATE with legacy save-set detection. Sanitized host migration and all3
settings fixtures pass; production settings preview pending. Historical
release notes remain accurate for those older binaries. Continue Start screen,
DE backgrounds and categorized navigation after numbered validation checkpoint.

## 2026-11-15 — Actual DS Style Start presentation

Imported credited DE Start light/dark bitmap assets with standalone originals,
lossless3070-byte stream generation, every-pixel host comparison and halfword ARM
video-bus replay. Start and five-category settings routing share native actions;
all-board input tests pass. Native themes retain legacy tabs. Generic DSSMM pages
use exact DE stripes, independent text/art palettes and preserved carousel cache.
Source Chis link fits after size optimization of cold/text-heavy paths. New
mGBA Start/category renderers use fake recent/art only. Full parity unfinished.
592 migration-only candidate not distributed due stalled hosted run. Next clean
combined all3 candidate includes stable non-PIE host sanitizer setup.

## 2026-11-15 — Build594 delivered

All3 clean packages, standard local full verifier and hosted43722736343 pass.
Evidence build-594-validation.json. Root hardware-test now Chis594 with Start,
backgrounds/settings hub and SYSTEM migration. Superseded own588 local files;
owner confirms588 font good. Next transparent #/A–Z folder pack, global basename
matches, then true category children and remaining DE surfaces. No parity claim.

## 2026-11-15 — Global letter folders, size safeguards

Added deterministic transparent #/A–Z closed-folder pack in both exact sizes,
alpha PNGs, licenses and ready-to-copy SYSTEM ZIP. CUSTOM already matches global
basenames; no firmware/RAM cost. All54 production decoder sanitizer checks pass.
Added hard packed-image gates matching native board budgets, boundary tests and
594 byte margins in evidence. Source remains594 for current hardware image.
Owner confirms588 font works well. Continue categories and remaining DE parity.

## 2026-11-15 — Independent settings children

Games and Hardware now filter actual options, with shared rendered-row/input
mapping and bounded cursors. Native themes/general tab retain full settings.
Save returns to the category's first option and its label is centered. Production
routing/category/movement tests and mGBA Games/Hardware/scroll fixtures pass.
Chis RAM remains tight; investigate read-only backgrounds in existing native
font-resource area before expanding further. No new hardware image yet;594 frozen.

## 2026-11-15 — Profile/time/navigation and background memory budget

Added cached NAME.TXT UTF8 first line and independent software menu clock with
selectable editor; stronger corners, NOR B→Start, wrapped navigation preserves
SD/NOR choice. Game RTC unaffected; no powered-off wall time. Owner594 confirms
progress, not new-source hardware validation. Background streams now use native
reserved font-resource partition with unchanged original prefix/data_size and
native loader algorithms. ARM source pixels/halfword discipline and readonly
region checks pass, recovering GBA RAM for menu work. Next full same-source
all3 build with flash/resource/RAM gates, CI and new Chis hardware-test image.

Focused profile/editor/navigation sanitizers and actual ARM external/missing-tag
replay pass. mGBA time edit/accept and Chis NOR-back fixtures pass and inspected.
Begin clean same-source all3 packaging/hosted validation before replacing594.

598 all-board/local/hosted43725948594 validation passes; retained tracked evidence
but not distributed. Owner requests12-character name cap, then cancels name
editing/keyboard. Apply12-character readonly display only; Time remains the
selectable header item. Next clean numbered all3 set and hardware-test delivery.

## 2026-11-15 — Build599 current hardware candidate

All3 clean size/resource/RAM/integrity gates, standard local verifier and
hosted43726431002 pass. Delivered Chis599 in root hardware-test, retaining art ZIP.
Name editor canceled;12-character NAME.TXT display only. Time/editor/wrap/NOR-B,
stronger corners, isolated settings children and readonly resource background
storage included. Superseded own594 local files, evidence retained. No public
release or physical599 claim. Continue DE category/body/layout parity autonomously.

## 2026-11-16 — Crash recovery and portable plugin work

Recovered saved sources/candidate/private session files; restored the sole empty
interrupted test, confirmed no orphan compiler/emulator. Portable Pogoshell PLUG
lookup adds BIN/GBA/MB/MBZ plus safe RAM-entry reset, complete LZ77 validation and
bounded halfword transfers. Native/bundled SuperFW fallback preserved; no imported
EZ-FLASH driver. Focused three-board host tests, actual ARM reset replay and20
existing pack format checks pass, not physical gameplay/save validation. Safe
emulators→PLUG migration and filesystem tests pass. IGM compressed install is
byte-identical via host, ELF and ARM unpack replay. Unified settings IO tests now
cover unknown/legacy retention, repeated saves, rollback and oversized files.
Remaining user settings/recents/Clean list/background/parity work remains active;
no new candidate/release yet. Keep599 frozen until clean full/all-board gates pass.

2026-11-16 follow-up: automatic settings save on exit (failed save retains menu),
no Save button, display-only Clean list across game titles, and minor Start art
position/size correction. Recents record selection before launch regardless of
visibility; separate DE-compatible10-SD history and private native/NOR history,
merge external order, safe replacement/recovery and CRLF support. Fixed actual
native word-copy filename overread uncovered by new sanitizer test. Focused
settings/navigation/art/background/file-IO checks pass; preview builds. Freeze
for complete all3/ARM/mGBA/hosted validation before new hardware candidate.
Owner archive moved to ignored .release-assets/folder-art.zip; retain/attach on
all future releases. Remaining menu parity work continues after this milestone.

602 full verifier stopped before board builds: the extracted carousel cache
fixture lacked the new Clean-list declaration. Production Chis linking and
focused name tests had passed; fixed the fixture's default settings/prototype
and rerun it. No failed full run is counted as a pass. Freeze new source for
complete verification again before delivering a candidate.

Build603/sourcef28de96b all3/full local verifier/hosted43780888958 passed. Exact
IGM decompression/metadata replay all3, background/metadata/reset ARM checks,
and mGBA Start/SET hub inspected. Delivered numbered Chis603 hardware-test image,
manifest and guide; superseded own599 files after retaining historical evidence.
No physical603/plugin/save qualification or full parity claim. SD flash headroom
5632bytes; keep all3 budget gates. Owner full folder ZIP stays ignored in
.release-assets. Next: remaining DS backgrounds/footer/dialog/settings parity.

Post603: lossless H/V/Help backgrounds and procedural-source DS grid now feed
read-only DBG3 resource streams. DS menus use these backgrounds, carousel/footer
bars removed outside List/native themes, vertical art can reach y159. Existing
cache/page reuse and centered Info identity preserved. Full12-background host/
ARM pixel/readonly checks, artwork/cache/page checks and Chis diagnostic link
pass; mGBA H/V screenshots inspected. Resource growth2616compressed bytes only
is a preliminary figure; clean all3/full budget checks still needed.603 frozen.
Root shared-tool registry correction ba4c310 passes all4 CI43782162091; synced
normally. Continue remaining caption/font/menu/dialog parity after next gates.

613 all3/full verifier/hosted43782888780 and additional asset/art/cache/ARM/mGBA
checks pass. New numbered Chis613 hardware-test image supplied;603 evidence kept.
SD flash has3072bytes remaining. Synthetic full-redraw mGBA timing warrants
component profiling, not a physical-FPS conclusion. Continue performance and
remaining caption/font/settings/dialog parity; no full completion/hardware claim.

Owner603 feedback: gear corner spacing expands1px, labels shortened, exact comma/
5-sec unlock hint, and14px DSSMM highlight preserving submenu bottom frame/gap.
Focused geometry tests and Interface mGBA bottom-row inspection pass. GG now uses
SMS plugin/native emulator list with original GG flags and exact credited DE GG
icon; appends no-changes native sprite/palette entries. All3 plugin/header/icon
checks pass. GB/GG save regression tests uncovered a separate dotted-stem double
strip; fixed bounded .sav append and verified all3 legacy/backup/state names.
613 remains current until clean all3/full/CI gates on this source. Continue parity.

615 all3/full/hosted43785459391 passed but not distributed: added exact historical
dotted-name save fallback to preserve existing data before next candidate. Full
names remain preferred/new default; no old files moved. Focused all3 filename
priority/legacy-temp tests pass.613 current. Private mGBA component timing points
to background decode as a worthwhile next optimization; ROM-fixture timing is
not physical firmware FPS. Continue autonomous parity/performance work.

Background run-fill+native DMA prototype reduces same-scene mGBA BG cost~61%,
total~30%, with identical art cost. Adopted bounded DBR1 codec with no new buffers;
resource-only compressed growth907bytes. All12 source/ARM/actual-DMA mGBA pixels
and guards pass. Shared PNG reader and stronger artwork oracle avoid counting DS
gray panels as art; host filter/background tests pass. Full image gates next;
613 current. Physical FPS/save/runtime qualification and full parity not claimed.

Failed plugin/native emulator loads now rebuild overwritten SDRAM browser/NOR/
recent tables and invalidate artwork before showing the error. Production-handler
sanitizer test passes; save/recent failures abort before loading. Source freeze
and clean full-all3 validation next, including final UI/GG/legacy-save changes.

617 full/hosted stopped on missing static forward declaration in recovery path;
fixed it and the cache function extractor's prototype handling. Rerun full clean
all3 validation;613 remains hardware-test. No failed run is reported as passed.

618 source4fc627ac all3/full local/hosted43790527480 and supplementary host/ARM/
actual-DMA mGBA checks pass. Delivered new Chis618 hardware-test, manifest/guide;
613 history retained. Includes owner603 UI/hint, GG shared SMS+icon and save-name/
legacy data follow-ups plus lossless DMA backgrounds/error recovery. SD has1536B
headroom; no raised limits/new buffers. Physical qualification/parity incomplete;
continue caption/font/menu/dialog/performance work autonomously.

Corrected a manual prototype-counter transcription: LZ BG700922, not790922;
exact font-glyph comparison resolves a black panel-border overlap. RLE BG gain
~56%, total~30%. Historical notes retained with correction. Timing rectangle
clears before printing and exact-glyph reader added. Private IWRAM hot-code
prototype shows further improvement; not firmware-qualified or adopted yet.

Owner618 follow-up underway: identified SMS/PCE startup WAITCNT=0x14, added
validated-chain safe timing correction; physical exit still unqualified. Single
RECENT.TXT replaces private sidecar after successful commit. Unified prefs already
existed; exhaustive roundtrip found/fixed artwork-Off carousel ambiguity with
namespaced field. Other reported persistence loss not yet reproduced. Four-key
unlock/rootB/opaque Help-System-flash panels/compact rows/gear/caption edits need
clean all3/full gates before next candidate;618 stays frozen. See HANDOFF.

New native-style GG cartridge icon passes all3 sprite/lookup checks. Exact real
SMS/PCE ARM startup replay confirms narrowly patched0x4000 write. IWRAM renderer
placement passes12 actual mGBA source-pixel/guard scenes; synthetic total~41%
lower vs prior run-fill, not physical FPS. Next candidate/Test5 publication
explicitly authorized with folder-art.zip; freeze and full all3 gates next.

623 full/hosted43868144493 stopped at Lite's enforced16KiB stack reserve after
hot-code relocation. No candidate distributed. Cold576B settings text cache now
moves to EWRAM; minimum stack unchanged. New full-all3 gates required. After
Test5 publication owner requests adversarial cleanup/correctness/performance
audit, then continued porting; do not mix audit into release source.

624 clean all3/full local/nativeIGM-ARM and hosted43868556156 pass; exact source
hash/package/size/seals checked, all360 UI combinations/board survive roundtrip.
Chis624 delivered hardware-test; old618 superseded.16KiB stack minimum intact,
Chis17304Bactual; SD1024Bflashfree. Integration31ab1e88/43868982790 before Test5
publication; then download/retention evidence and adversarial audit. No public
release or complete physical qualification yet.

Test5 published at sourcea4cafcaf/Build624 after all3 local/hardwareCI and all4
integrationCI pass. Chis/SD/Lite+full folder-art.zip downloaded and byte-verified;
notes match. Test4 page/assets pruned after exact archive, tag retained; BLUE/
OMEGA sole prereleases and stable DElatest unchanged. Evidence publicationJSON.
Next requested adversarial source audit, then continued autonomous DE parity.

Post-Test5 audit first pass corrects parent-path overflow, malformed/failed recent
loads, benchmark zero divisor and byte-compare portability; removes old save alias,
duplicate guard and orphan QR assets. Ten active DBR2 backgrounds match all pixels
host/ARM/mGBA; blob25332Bsmaller. Scaler integer stepping matches division-oracle
pixels and cuts synthetic art42%/total20%.40-source forcedCppcheck audited. Full
all3 gates still pending; next patch scanner bounds/capacity/chunk/IO audit, then
continued parity.624 release/hardware-test remain frozen; no physical claims.

Audit second pass fixes scanner read/capacity/group-order bugs with explicit
lookbehind/lookahead windows; exact allocations, whole/window equivalence and
production9MiB+3 fault/cleanup tests pass. NDSnull/shift/handle bugs fixed with
synthetic tests. LateCppcheck diagnostics corrected earlier incomplete warning
summary; third-party byte-casts/cache-copy explained. Extra scanner code needed
256B cold filename relocation to EWRAM; preliminaryLite stack16592>=16384.
Freeze clean source for full-all3 gates/new hardware test, then resume DE parity.

632 clean full/all3/nativeIGM-ARM and hosted43874378364 pass; source/package/budget
checks pass. Delivered Chis632 hardware-test, retained release624. SD4608Bflashfree,
Lite20480B, Chis425472B. Audit next: IGM cheat-size omission/unchecked subtraction,
GBA short reads/slow-copy tails and failure asset-loss limits. Do not claim audit
complete or generic GBA recovery yet. Then continue porting without input pause.

Audit3 in progress: bounded shared IGM/cheat planning, exact native fast/slow ROM
transfers and explicit restart-only screen after destructive load failures.
Host32MiB hole/tail/fault tests, all3plugin tests and mGBArestart preview pass;
full gates pending. Owner624 queue now includes lined/framed Help/System/update,
vertical System centering, SuperFWlogo, shorter/wider/blankGG and missingicons,
NORfooter, Start/time corners, true horizontal art clipping, unresolvedpowercycle
prefs, PCEloadfailure, A26/CRTrecognition and Chis bottom Load/WriteNOR selector
with capacity/erase confirmation flow. Details/currentstate inHANDOFF; no inputpause.

Added requested Chis GBA Load/WriteNOR bottom selector, capacity/per-game checks,
default-No write/replace-all confirmations and source preflight. Existing backend
now handles32MiB/no-patch, exact reads, odd-byte padding and close cleanup. Mock
NOR control/backend tests pass without physical writes. Audit3 common loader and
settings read diagnostics/FatFs FAT16/FAT32 remount gates added. Exploratory full
verifier stopped before builds to include NOR fixes; clean full gates pending.

First clean audit3/selector gate failed the SD compile: devkitARM uint32_t is
unsigned long, while three helper output locals were unsigned int (host typedefs
masked mismatch). Corrected locals to uint32_t; no firmware delivered from634.
Rerun full clean gates on next committed source; preserve this failed result.

635 corrected SD compilation and passed host/restart-preview checks; local full
run intentionally stopped during SD packaging after review found another NOR
preflight issue. Tighten payload availability to include trampoline space and
checked/aligned hole bounds before replacement confirmation. Honor allocator
failure before calling writer (avoids using an incomplete block map). Added real
availability-function regression tests; final clean gates on636 next. No635 image
distributed; hosted635 result tracked separately from final636 gates.

636 full local/nativeIGM/all3/integrity/hosted43885295944 passed. Before delivery,
removed mutation of normal Load DS/IGM options during NOR attempts; canceled or
failed NOR writes retain the original launch configuration. Mock regression
passes. Final637 frozen gates next;636 evidence preserved, no physical claims.

638 correction chunks native fills within DMA16-bit count;2MiB NOR scratch tests
use actual helper signature and pass.637 local run intentionally stopped, no
637 image delivered. Owner authorizes next validated release; prepared Test6 notes
with immediate predecessor folded into concise past summaries. Clean final gates
and integration/publication still pending; preserve Test5/624 until replaced.

638 full/all3/nativeIGM ARM, host regressions, package integrity and hosted
43886442413 passed. Chis638 delivered hardware-test; report build-638-validation.
Native flash limits unchanged: SD3584B/Lite19968B/Chis425472B headroom. Next integrate
normally, all4 CI, publish authorized Test6 and archive/prune Test5, then porting.

638 hardware notes incorporated: lined shared panels/header-safe modal clearing,
Help logo, System centering, NOR List/footer, bracketed plain launch selector,
Start corners, narrower horizontal text and revised blank GG cartridge. Host
source-pixel/guard and mGBA Help/ROM/System previews pass. Canonical-only settings
ignore UI/US sidecars, save changes/exits, read back and roll back failed commits;
corruption/roundtrip/FAT16/FAT32 checks pass. Raw MB odd tails accepted with padded
reset size; all3 plugin tests pass. Physical settings/PCE/plugin Exit remain open.
No638 release yet; publish next validated correction per standing owner approval.

640 clean full/all3/nativeIGM ARM and hosted43978497350 pass. Package integrity
checks pass; Chis640 hardware-test delivered, replaces638 localcopy. Panel/settings/
plugin regressions and real mGBA previews recorded in build-640-validation.json.
Next normal integration/all4 gates and authorized Test6 publication, then porting.

Test6/Build640 published after local/all3/nativeIGM and all4 integration gates.
All4 downloaded assets/body match; folder pack attached. Archived/pruned Test5
release while retaining tag; BLUE/OMEGA/DE untouched. PublicationJSON records
source/CI/hashes/limits. Next universal release docs/sync/cleanup then porting.

Owner640 hard-freeze changing Interface options; Test6 withdrawn/deleted by owner.
Immediate save-on-change removed locally; investigate actual ARM save path and
stack/bus constraints before next candidate. Save-on-exit remains. No fix claim,
no640 republication. Other porting deferred for this critical regression.

Actual ARM reproduced bad%s pointer from Thumb1 IPA-RA return clobber. Global
SuperFW -fno-ipa-ra fixes native save/load; stack6760B. Actual compiled menu/FatFs
mGBA tests pass change/save/native reset and separate-process persisted boot.
Fixture sources/generator/runner/recovery docs added to repository; verifier now
includes it. Frame149 bevel protected and screenshot-checked. Full gates/PR/new
candidate next, no physical qualification claimed.

647 all3/full/nativeIGM and actual linked ARM + mGBA persisted-boot/frame checks
passed. Chis647 delivered hardware-test;640 withdrawn. Native GCC control fails,
workaroundpasses. Hosted firstpush missingXvfb only; sharedCI deps fixed770f783.
Next hardware PR/checks/mergecommit/all5sync, preserve fixtures andprivatefiles.

PR9 mergecommit integrated correction/fixture after all4CI43985535980 + push
43985527343 passed. Hosted nativeARM/mGBAtwobootproofconfirmed. Isolation5/merge3
androotpolicy pass; no otherkernel source edits. Chis647current; Test6withdrawn.
Next normalrootmilestone/branchsync and continued nativevisual/porting checks.

Native mGBA9-screen visual matrix now catches stale-art and frame-overlap pixels.
Found/fixed popuphelp lower descenders(4pixels) and Orangebottom-rowg(2pixels);
all9dirtyELFscenes pass. Expandedfixture/DWARFtype offsets committednext; full
all3 gates stillpending. 647current/frozen, no newdirtycandidate delivered.

657 clean full/all3/nativeIGM/ARM andmGBA2boots+9visualscenes passed; hosted
43989337269success, packet/budgetcheckspass. Chis657deliveredhardware-test,647report
retained. Next visualslicePR/checks/normalintegration/sync; Test6withdrawn.

PR10 merged after all4CI43990263281 pass. Native9-scene regression retainedmain;
657currenthardware-test. Normaldoccheckpoint/all5sync next, then porting.

Expandednativevisualchecks tobothfontchoices; font0had6popupdescenderoverlaps.
Font-specific action/help/unlock baselines corrected;18scene checks inprogress.
657stillcurrentvalidatedcandidate; no dirtybuild delivered.

2026-11-17: Owner confirms Build 657 settings persistence on Chis. Implementing
remaining 657 menu feedback, optional 12-hour display and unchanged-save avoidance.
Expanded native regression distinguishes accent colors from completion markers and
checks frame bounds throughout animation. Clean all-board validation pending.

664 validation and PR11 integration completed successfully. Owner tested664 and
provided next UI/background-save corrections. No release published; new normal
release authorized after new gates. hardware-test retains validated Chis664.

Independent Top bar color (default gray/black, colored bars/white), synchronized
clock x234 and shorter illustrated GG cartridge are implemented. Cooperative
verified settings transactions pass host faults/interleaved reads/requeued changes
and actual ARM replay; native mGBA navigation-while-pending and two-boot persistence
pass. Full all-board/IGM/18-scene/hosted gates next; keep664 until replacement passes.
