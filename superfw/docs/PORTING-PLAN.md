# DSSMM on SuperChis Prime v1.0 — porting plan

Status: design proposal, 2026-11-12. Owner has a physical SuperChis Prime v1.0 available for testing. No physical result has been reported yet. Firmware baseline: SuperFW dabbf4909f85e556f7a12972cc8a7b94fc54ac51.

## Approach

Keep SuperFW as the firmware foundation and implement DSSMM presentation and navigation within this project. Prioritize the `chis` board; retain build compatibility for `sd` and `lite` without promising identical feature sets. All implementation stays under superfw/ on its lane. Significant work is expected in the menu, rendering, artwork, configuration and emulator integration; shared repository infrastructure already supports the project.

Do not transplant OMEGA drivers, patch payloads, save-state formats or flash layouts. SuperFW already owns SD/filesystem access, WAITCNT/save/IRQ patching, direct saving, game launch, NOR mapping and firmware updates. Keep these paths intact during the first UI milestones. Introduce small interfaces around menu actions and browser metadata so the renderer does not issue hardware operations directly. Existing src/menu.c mixes UI and actions, so extract incrementally rather than rewrite it wholesale.

## Evidence and open hardware questions

- Upstream Makefile enables SUPERCHIS_IO, SUPPORT_NORGAMES, extended fonts and bundled emulators for `chis`, with a 2 MiB firmware limit. src/supercard_driver.c implements SuperChis NOR mapping and SRAM banking; src/loader.c has native NOR launch/write paths. This is a substantial starting point, not proof of Prime v1.0 qualification.
- The linked SuperChis repository describes a CPLD implementation, memory mapping and SD interface. Its README does not identify the owner’s exact Prime v1.0 revision or prove compatibility with our pinned firmware. Record installed firmware, board markings, console model, battery status, reported flash/SRAM identity and any vendor-specific recovery instructions before writes.
- Sources: https://github.com/ChisBread/SuperChis and https://github.com/davidgfnet/superfw . Preserve David Guillen Fandos’s firmware attribution and ChisBread’s hardware attribution. SuperChis HDL is CC BY-NC-SA 4.0; consult it as protocol documentation and do not copy HDL into the GPL firmware. Audit DSSMM code/assets individually before reuse; preserve DS Style/Merkin Mix and third-party credits.

## Feature disposition

| DSSMM feature | Planned treatment |
| --- | --- |
| DS-style panels, icons, typography, Light/Dark themes and accents | High visual fidelity is feasible on the same 240×160 GBA display. Adapt assets/rendering after license, palette, VRAM and firmware-size checks. |
| List and List + Art | First UI targets; use SuperFW browser and launch actions. |
| Horizontal/vertical carousels and transparent artwork | Later renderer work; prototype RAM use, image decoding and SD latency before promising parity. Preserve DSSCC name-matched artwork compatibility as specified below. |
| Favorites, recents, last-played and startup shortcuts | Carry over behavior; retain native recent/save lifecycle and add versioned settings where needed. |
| Menu sounds and localization | Feasible, but audit audio/interrupt ownership and preserve SuperFW UTF/font support. |
| Clock-based Auto theme | Omit on Prime. Keep manual Light/Dark themes and accent selection; do not add a software clock solely to reproduce automatic themes. |
| GBA loading and save management | Preserve native patch engine, battery SRAM recovery and direct-saving modes. Validate each save technology and power-cycle recovery. |
| Cheats and remembered selections | Present DSSMM-like controls over SuperFW’s engine. Audit code formats and per-game identifiers; no assumed database compatibility. |
| In-game menu and save states | Retain SuperFW mechanisms and their game-dependent limitations. OMEGA state files/runtime payloads are incompatible unless separately demonstrated. Style this small runtime menu later within its own memory limits. |
| NOR library and themed progress | SuperFW chis already has NOR support. Adapt UI around native allocation/mapping/write/erase; do not reuse OMEGA sector maps. |
| MiGoomba and other emulators/plugins | Separate integration project. First preserve bundled upstream emulators. Audit append/load conventions, save paths, return/reset and memory limits before MiGoomba adoption. OMEGA plugin binaries have no blanket compatibility. |
| OMEGA-specific rumble, cartridge modes and firmware/FPGA update flows | Omit rumble, hardware-clock UI and unsupported OMEGA cartridge modes on Prime. Keep only native SuperFW update and hardware controls supported by the target. |

## Visual target

Aim for a closely recognizable DSSMM main browser: matching overall composition, artwork placement, selection treatment, colors and navigation conventions. Exact pixel parity is a design target only after asset/rendering measurements. SuperFW-specific patch/save controls, tools, updater and hardware information need their own screens using the same visual language. In-game menus can share colors and typography but may need a simpler layout. Do not quote a numerical similarity percentage before a rendered prototype exists.

## Milestones and acceptance gates

1. **Prime baseline:** confirm board identity and recovery route; back up SD contents and saves, and firmware where supported. Prefer a documented chain-load route if the installed firmware/board supports it. Test unchanged chis build first: boot, SD browse, one disposable ROM and save, reboot recovery. Keep firmware/NOR writes out of the initial smoke test. User performs physical tests and records results.
2. **UI seam and first skin:** separate browser model/actions from drawing in small steps. Add a selectable DSSMM theme with list mode, preserving the native menu as a recovery/debug option. Capture screenshots plus boot and navigation timings; check linker/map/stack budgets and firmware size.
3. **Artwork and navigation:** List + Art, favorites and settings, then carousels/transparency. Test huge directories, long UTF-8 paths, missing/corrupt artwork, rapid scrolling and SD read failures. Demonstrate responsiveness on the physical card.
4. **Game/save qualification:** SRAM, EEPROM and Flash games; direct saving and battery-backed recovery; cheats, reset/return and in-game menu enabled/disabled. Back up saves and compare actual persisted data. Record failures per game and patch database version; do not generalize success from one title.
5. **NOR and emulator expansion:** after recovery and baseline saving pass, exercise native NOR write/launch/delete and storage-full behavior using disposable content. Add MiGoomba only after a separate save/launch compatibility review. Never interrupt physical flash writes merely to simulate faults; use software fault injection for destructive scenarios.
6. **Candidate:** host tests, all three board builds, isolation checks when architecture changes, and physical regression matrix. Release initially as a Prime-focused prerelease with exact tested revision/console/card details and explicit untested variants. Implementation authorized by the owner after this planning checkpoint; release publication requires a qualified candidate and owner direction.

## First implementation slice

Inventory menu rendering, theme tables, resource sizes and DSSMM asset licenses; define browser/render/action interfaces; produce a list-mode DSSMM skin without changing patch/save/flash code. Before distributing the first hardware candidate, obtain the owner’s installed firmware/recovery details. Keep the pinned baseline available for A/B testing and record every physical observation in a dedicated validation log.

## Designer-thread supplement

Source: [ChisBread’s Prime introduction, January 12, 2026](https://gbatemp.net/threads/superchis-prime-how-i-fixed-the-supercard-sds-flaws-on-a-budget.678808/), reviewed November 12, 2026. These are designer statements, not our hardware results.

Prime combines NOR execution with retained SDRAM and fragmented flash allocation. Test both launch paths separately; make the NOR library a first-class UI destination. Flash-chip variants and reported GBA motherboard-02 power issues make chip/console identification part of qualification. Standard saves use battery-backed SRAM; EEPROM/Flash handling depends on firmware patches, with ROM hacks needing particular attention. No hardware RTC or rumble is provided. The designer identifies SuperFW support, strengthening the native-backend choice without qualifying our build or the owner’s revision. The linked HDL repository is described as the original SuperCard implementation, so it must not be treated as a complete Prime hardware specification.

## Owner scope decision — omit unsupported hardware features

Automatic clock-based themes are excluded from the Prime port. Retain manual Light/Dark themes and accent colors. Omit controls for absent rumble, hardware RTC and unsupported cartridge modes rather than display disabled placeholders or emulate hardware solely for menu parity. Preserve SuperFW’s existing software RTC game-patching support where applicable; that is independent of menu theme scheduling. Scope the visible interface to target capabilities, including variant-specific builds. Visual consistency means a common design and navigation style, not identical feature counts across cards.

## DSSCC artwork compatibility requirement

Owner requests retaining pack generation with [DSSCC](https://github.com/voluptuary/dsscc). Adopt its existing output as an input compatibility target, rather than require a Prime-only artwork generator. DSSCC documents 120×80 title images under IMGS, 80×80 box images under IMGS2, exact-name files/folders in CUSTOM, and EZ-FLASH-compatible 15-bit BMP output. Its documented CUSTOM index limit is 256 entries per folder; do not silently truncate or assume this is the eventual Prime limit.

Before implementing the artwork milestone, pin/review the generator source and existing DSSMM lookup implementation: filename/extension rules, case handling, mirrored directories, index encoding, transparency markers, fallback precedence and BMP header/pixel details. Preserve existing pack layout and matching behavior where supported; translate pixels in the Prime renderer if necessary. Keep artwork external on SD, load on demand and bound caches to avoid inflating firmware or exhausting RAM. Name matching is independent of which emulator launches the ROM.

Acceptance: build a representative pack with unmodified DSSCC, copy it to the Prime SD card, and verify exact-name title/box artwork for GBA and supported non-GBA entries. Cover duplicate names across directories, Unicode/long names, absent/corrupt files, transparency and navigation latency. NOR entries should retain their source artwork identity where native metadata permits. No compatibility claim until these fixtures and physical rendering pass. This checkpoint adds a design requirement; the current imported firmware has no DSSCC artwork reader yet. Credit DSSCC and its FrankieT19 scraper lineage in implementation documentation.

## Implementation checkpoint — first manual palettes

Started 2026-11-12 by owner request. Appended two original DSSMM-oriented Light/Dark palettes without changing upstream IDs 0–5 or saved numeric settings. New chis configurations default to Light; existing theme settings override it. Other board defaults stay upstream. UI settings identify the new choices as Light/Dark; both use existing theme persistence and rendering, including native in-game palette propagation. This is a palette foundation, not the completed DS-style layout. Independent accent selection, artwork and browser restructuring remain pending. Existing numeric themes remain available for comparison. Physical contrast/readability needs testing on the owner’s console.
