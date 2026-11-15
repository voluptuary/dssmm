# SuperFW project instructions

This project targets SuperCard GBA flash carts and derivatives/clones. Its codebase is upstream SuperFW by davidgfnet, pinned and documented in `docs/UPSTREAM.md`. Read root `AGENTS.md` and this project's `HANDOFF.md` and `PROGRESS.md` before project work.

On branch `superfw`, edit only `superfw/`, including its journals. All repository branches retain the complete shared tree; never remove other project directories. Merge this lane normally into `dssmm`, then synchronize `dssmm` back to this lane. Integrate through `dssmm`, not another hardware branch. Root build/CI/policy changes belong on `dssmm`.

This project has three upstream board variants: `sd` (default SuperCard SD), `lite` (SuperCard Lite), and `chis` (SuperChis). Build one with `make superfw BOARD=sd|lite|chis`; run `make verify-superfw` for all three builds and the upstream host tests. Build outputs stay in `superfw/build/`. Do not assume the variants or their firmware are interchangeable.

Preserve upstream copyright headers, `COPYING`, component notices and attribution. Most firmware source is GPL-3.0-or-later; individual components retain their own licenses. Do not move SuperFW code into another kernel directory or relabel its license. The upstream firmware is intended for SuperCard-family hardware; DSSMM's integration and this repository's builds still require independent validation. Do not claim physical hardware testing without recorded evidence.

## Active Prime port continuation

Owner authorized implementation, starting with easy presentation changes. Read docs/PORTING-PLAN.md. Prime omits Auto themes and absent hardware controls; manual Light/Dark remain. Preserve DSSCC pack compatibility as an implementation target. First code checkpoint adds palettes at IDs 6/7, retaining upstream IDs 0–5 and existing saved settings. New configurations on all three boards default to DSSMM Light. Run bash scripts/test-theme-settings.sh for parser/default regression checks; scripts/verify.sh includes it. Build board variants sequentially: upstream make clean and intermediate outputs are shared inside this project. Independent accents are now implemented through menu_theme_colors() and persisted as accent=0..4 (0 preserves native colors). Artwork lookup/preview is implemented for mirrored and CUSTOM name-matched files; continue expanding supported DSSCC fallbacks and coverage. All three boards default to Title artwork and UI settings exposes List/Title/Box; pre-Test-2 unversioned native defaults migrate while Test 2 versioned choices remain. The owner wants all three board variants kept in sync wherever hardware permits, with consistent DSSMM skinning across browser, settings, system/tools, game screens and in-game menus as a top goal. Continue screen by screen without changing SuperFW action/backend paths. New labels remain English for later localization. Do not claim completed DS-style parity or physical validation without evidence.

Artwork continuation: decoder src/artwork.c/.h and exact-read adapter artwork_file.c now feed the SD-browser prototype. scripts/test-artwork.sh runs sanitized decoder/adapter/compositor checks without PIE. Halfword-aligned caller storage is required; output is invalid on failure. Palette20–235 is uploaded by theme reload. Fixed cube quantization loses precision. Optional real-generator crosscheck: python3 tests/test-dsscc-writer.py /path/to/dsscc (pin in docs/DSSCC-FORMAT-AUDIT.md). Read docs/ARTWORK-PREVIEW.md for currently supported paths; do not claim full pack/hardware compatibility.

Current artwork checkpoint: SD browser Start cycles List/title/box; save via UI settings. Read docs/ARTWORK-PREVIEW.md. artwork_file.c checks CUSTOM filename stems before mirrored filename stems; Title mode then reads GBA header game code at 0xAC and uses `/SYSTEM/IMGS/<first>/<second>/<CODE>.bmp`. Reads are exact FatFs reads. Dedicated halfword-aligned preview storage is in cartridge SDRAM state; never add a byte-write cache there. Decoder requires aligned output, and artwork_blit_frame handles Mode4 halfword composition. Scripts test adapter/decoder/compositor and settings. Hardware visuals/cache/gameplay remain unqualified; NOR/recent previews and unrelated artwork fallbacks remain unsupported. Earlier “not wired” entries are historical and superseded by this checkpoint.

Published checkpoint: superfw-test1 is a prerelease at source8a22da16c2d755de8fc12db7c5482657e6813ecd, with chis/sd/lite .fw assets. Owner reports successful chain-loading and manual themes on one SuperChis Prime v1.0 (SuperFW 0.22); this is a boot/theme smoke test only. SD/Lite, gameplay, saves, artwork, NOR and installation remain untested. The menu is still mostly upstream SuperFW; DSSMM-style layout work remains. See docs/prime-test1-hardware-observation.json, docs/releases/superfw-test1-publication.json and docs/ARTWORK-SETUP.md. Preserve tag/assets. Native version0.22 and upstream bundled emulators remain; no MiGoomba in this port. Root README/downloads are end-user facing. Owner authorized correcting the release notes to reflect this evidence; preserve the archive of the prior body.


Build identity: the Info page shows `SuperFW <version> (<8-digit upstream build hash>)`, then `DSSMM Build N`. `N` is `git rev-list --count HEAD` over full repository history; kernel CI fetches full history. Override with `DSSMM_BUILD_NUMBER=N` only for controlled reproducible builds. Preserve `VERSION_SLUG_WORD` in the firmware header as the upstream commit identifier; it is also displayed after the SuperFW version.


SYSTEM storage (owner update): all `.superfw` resources now target `/SYSTEM`.
Read docs/SYSTEM-LAYOUT.md. Startup migrates legacy contents before pending SRAM
handling, merging directories without overwriting files; conflicts/errors stop
boot for manual resolution. Directory settings are retired; new saves/states use
fixed SAVEGAME/SAVESTATE. Preserve automatic legacy save reads to avoid data loss.
Earlier preserve-.superfw instructions are superseded by the explicit owner
request. Run test-system-layout.sh plus all-board settings/launch checks.


## Workspace cleanup

Before handoff, remove stale ignored compiler outputs and superseded local board/release builds after validation (`make -C superfw clean`, then remove `superfw/build/`). Preserve tracked upstream assets, journals, validation evidence, published tags and GitHub release assets. Never remove user-created untracked files; check `git status` first.

## Current synchronized release

`superfw-test4` is the current SuperFW prerelease for all three boards. SD, Lite and Chis share DSSMM Light/Title defaults, artwork controls, browser styling and SYSTEM paths; version-2 Test 2 preferences are preserved during migration. Chis NOR remains hardware-specific. Tag source, CI, local builds, asset hashes and no-Test-4-hardware-validation statement are in `docs/releases/superfw-test4-publication.json`. Only the newest prerelease page is retained per SuperFW, BLUE and Original OMEGA lane; superseded release bodies/assets are archived at the root and git tags remain. DE `v8.4.0` remains latest stable. Continue workspace cleanup and record physical checks per board.


## Latest artwork/UI follow-up (2026-11-13)

Owner reported that artwork did not display with GBA games grouped under letter folders. Keep lookup order exact: Title checks CUSTOM by ROM filename, mirrored ROM filename, then the GBA game-code path `/SYSTEM/IMGS/<first>/<second>/<CODE>.bmp` from header offset 0xAC; Box checks CUSTOM then mirrored ROM filename only. Both use /SYSTEM/IMGS or /SYSTEM/IMGS2 respectively. Remove the browser artwork frame and top-bar DSSMM text; align option lists/settings left. Font glyphs are fixed height in the existing renderer; no font-size setting is supported. Validate all three boards and hosted suite before release. Owner requests retaining only the newest prerelease for each of SuperFW, BLUE and Original OMEGA after publishing the next build; preserve DE stable.

## Active parity goal (2026-11-14)

Owner requests autonomous work until the SuperFW builds reach DSSMM visual/interaction parity with the EZ-FLASH DSSMM kernel, including fluid carousel views. Use `ezfomegade` as the primary visual reference (the owner calls it EZFODE),
with `ezfomega` as a supporting reference only; port presentation and behavior, not its private card drivers/protocols. Keep all three SuperFW board variants synchronized where hardware allows. Start with standalone rendering/animation seams and preserve native SuperFW actions. mGBA 0.10.1 is installed locally; it can exercise GBA CPU/video and debugger paths but does not emulate SuperCard storage/card protocols. BIOS is optional for many homebrew paths. Do not claim card hardware validation from emulator output.

Owner Test 4 feedback: Box artwork appears slowly, Title ROM-header art does not. The candidate loads each bounded BMP with a word-aligned bulk FatFS read plus a cached tail read into an aligned 19,254-byte SDRAM buffer, supports top-down/bottom-up 16-bit BMPs with zero size fields, and tries flat game-code `/SYSTEM/IMGS/<CODE>.bmp` after the nested EZ path. Focused artwork/settings/motion suites pass. `lcov` and `libssl-dev` are installed; the upstream host suite passed once standalone but integrated verifier reruns intermittently hang with AddressSanitizer `DEADLYSIGNAL` in `cimpl_test`. Do not suppress or claim a clean full run. UI settings v4 persists List/Horizontal/Vertical browser mode. `browser_carousel.c` is shared by firmware and the mGBA schematic preview ROM. Full menu parity, physical title art, box-art timing and card behavior remain unverified.

Latest renderer slice aligns carousel artwork geometry with the EZ-FLASH layout: horizontal center 120×80 with 60×40 neighbors; vertical center 84×56 with 48×32 neighbors. `artwork_blit_scaled()` supports the smaller vertical viewport with checked even-width Mode 4 writes. Removed the trial card outlines and fixed the old right-edge clipping. Host artwork/settings/motion tests and clean SD/Lite/Chis builds pass. Read the latest entries in HANDOFF/PROGRESS before changes; full upstream ASAN suite is still intermittent locally.

The isolated mGBA preview now renders the generated Title artwork in packaged mGBA 0.10.1 (`/usr/games/mgba`). Run `bash scripts/test-browser-preview-mgba.sh` to rebuild the fake-ROM preview, capture a headless screenshot and assert its known artwork marker. This does not exercise FatFS/card paths or establish full-kernel/hardware behavior. Do not use a plain `mgba` command unless `/usr/games` is on PATH.

Track every candidate using docs/BUILD-TRACKING.md and generated board build-info.json. Commit before producing test images; preserve displayed number/hash plus source fingerprint and image digest in validation evidence. Build 534/hash126ae458 was a dirty-source carousel candidate; its hardware crash/missing-art/sluggishness report is not resolved by synthetic tests.

## Carousel memory rule (2026-11-15)

All carousel cache metadata (flags, path keys and artwork_info) MUST stay in
internal GBA RAM. Only decoded halfword pixel arrays belong in cartridge SDRAM;
ordinary byte writes there are invalid. Every board build checks the linked
metadata address. See docs/CAROUSEL-ARTWORK-MEMORY.md for the ARM regression that
fails on owner-tested Build553 and passes with the fix. Owner confirms List Box
art works and edge crash is fixed, but corrected carousel art still requires
physical retesting. Owner has completed the model switch and authorized continuing the port after
delivering the Chis candidate. Keep Build561 hardware-test image unchanged while
awaiting physical results.

Current continuation: read docs/UI-PARITY.md and latest journals. Filename carousel
art is physically confirmed on Chis561; Title code-art and585 performance remain
unconfirmed. Current local hardware-test image is Chis585/hash45e4dbbc. Keep it
frozen while later UI work proceeds. Owner explicitly requires autonomous
continuation without a permission/model-switch pause.

BMP staging is a separate cartridge-bus constraint: requests directly into SDRAM
must use word-aligned buffers/counts. Read the final1..3bytes into GBA RAM and
copy them with halfword stores; do not restore a whole-file memcpy tail there.
The optional ARM adapter test reproduces a byte-store fault in the old code.

## Build582 owner follow-up (2026-11-15)

Header Title only renders AGS/TCHK; see docs/HEADER-ART-COMPATIBILITY.md and
historical fix944ac13 (planes0, not DST1). Match native bounded opaque code-record
semantics, keep filename parsing strict and retain CUSTOM priority. Owner removes
the Animation toggle/instant-mode feature: normal carousel transitions remain,
old config keys are ignored, new settings version6 omits the key. Cache5 images
(selected±2), prioritizing visible3 and avoiding redraw/hot-copy for outer loads.
Center the SuperFW version/hash Info line until full DE styling. Produce numbered
Chis hardware-test builds after all3 variant builds/gates; continue autonomously
without requiring owner feedback. Owner will identify tested build numbers.

## Owner585 confirmation and new focus goals

Owner physically confirms Chis585's ROM-header images and five-image cache work.
Prioritize docs/FOCUS-GOALS.md: CUSTOM directory artwork in Title/Box, then an
optional credited DE DSSMM system font. Native fallback, byte-safe rendering and
all3 variants remain required. Indexed filesystem work is deferred until these
priorities are implemented; no such library/source change has been made.

Font continuation: UI Font SuperFW/DSSMM is optional, native default; read
FONT-PORT.md. Preserve credited Apache asset, reproducible conversion and native
Unicode fallback. In-game font uses reserved palette word5, not a new payload
field. Run test-dssmm-font.sh alongside settings/artwork/bus tests. Current folder
CUSTOM loader is implemented for List+Art and carousels; full basename including
dots, no header lookup for directories. Both features need physical retesting.

Owner continuation: work continuously without awaiting feedback until stopped or
DE parity is reached, supplying numbered hardware-test builds at validated
milestones. Read LAUNCHER-PORT.md and current UI-PARITY map. Start/category hub and
lossless DE backgrounds are implemented but category children/dialogs remain
unfinished. Never claim100% parity based on presentation fixtures. All3 variants
share launcher features; SD/Lite substitute Recents for unsupported NOR. Keep
palette0..15 backgrounds separate from UI16..19/artwork20..235 and Info logo.
Host sanitizer executables now disable PIE to avoid recurrent ASAN startup loops;
sanitizers remain enabled. Firmware optimization flags are separate.
