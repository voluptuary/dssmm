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


Build identity: the Info page shows the current SuperFW version followed by `(DSSMM)`, then `DSSMM Build N`. `N` is `git rev-list --count HEAD` over full repository history; kernel CI fetches full history. Override with `DSSMM_BUILD_NUMBER=N` only for controlled reproducible builds. Preserve `VERSION_SLUG_WORD` in the firmware header as the upstream commit identifier; it is not the user-facing build number.


SYSTEM storage: new save files use `/SYSTEM/SAVEGAME/` or `/SYSTEM/SAVES/`; disk states use `/SYSTEM/SAVESTATE/`; the debug log uses `/SYSTEM/superfwlog.txt`. Preserve `.superfw`. Legacy `/SAVEGAME`, `/SAVES`, and `/SAVESTATE` data is detected and continues to be used for matching games when no SYSTEM copy exists; never remove these fallback reads without a verified migration and regression coverage. See `docs/SYSTEM-LAYOUT.md`.


## Workspace cleanup

Before handoff, remove stale ignored compiler outputs and superseded local board/release builds after validation (`make -C superfw clean`, then remove `superfw/build/`). Preserve tracked upstream assets, journals, validation evidence, published tags and GitHub release assets. Never remove user-created untracked files; check `git status` first.

## Current synchronized release

`superfw-test3` is the current SuperFW prerelease for all three boards. SD, Lite and Chis share DSSMM Light/Title defaults, artwork controls, browser styling and SYSTEM paths; version-2 Test 2 preferences are preserved during migration. Chis NOR remains hardware-specific. Tag source, CI, local builds, asset hashes and explicit no-Test-3-hardware-validation statement are in `docs/releases/superfw-test3-publication.json`. Preserve Test 1/Test 2 releases and DE `v8.4.0` latest stable. Continue workspace cleanup and record physical checks per board.


## Latest artwork/UI follow-up (2026-11-13)

Owner reported that artwork did not display with GBA games grouped under letter folders. Keep lookup order exact: Title checks CUSTOM by ROM filename, mirrored ROM filename, then the GBA game-code path `/SYSTEM/IMGS/<first>/<second>/<CODE>.bmp` from header offset 0xAC; Box checks CUSTOM then mirrored ROM filename only. Both use /SYSTEM/IMGS or /SYSTEM/IMGS2 respectively. Remove the browser artwork frame and top-bar DSSMM text; align option lists/settings left. Font glyphs are fixed height in the existing renderer; no font-size setting is supported. Validate all three boards and hosted suite before release. Owner requests retaining only the newest prerelease for each of SuperFW, BLUE and Original OMEGA after publishing the next build; preserve DE stable.
