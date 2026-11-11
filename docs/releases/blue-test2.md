# THIS BUILD IS FOR THE NEW OMEGA BLUE ONLY!  UPDATE AT YOUR OWN RISK!

# Changelog

## DS Style Merkin Mix 8.3.1 — BLUE Test 2

> **EZ-FLASH OMEGA DEFINITIVE EDITION B / BLUE ONLY — HARDWARE UNTESTED.**
>
> This is an updated experimental BLUE port following the cross-platform software audit. Source, build, host tests and linked ARM/Thumb checks have passed, but it has **not been installed or tested on a BLUE cartridge**. The larger kernel layout preserves the vendor's fixed resources; acceptance by the unpublished BLUE updater remains unverified. Booting, gameplay, saving, plugins, cheats/save states and NOR writing all need hardware verification.
>
> **Do not install on the older OMEGA Definitive Edition Rev. A or Rev. B, the original OMEGA, or any other cartridge model.** The separate Definitive Edition 8.3.1 release remains the stable release for Rev. A/B.

### Fixes in Test 2

- Made NES detection and patch handling safer for incomplete or malformed input, including games being written to NOR.
- Added a final capacity check before patch writes can select or modify cartridge memory.
- Corrected valid Chinese filenames being mistaken for folder separators when saving, creating backups or finding artwork.
- Tightened validation of marked transparency artwork while retaining ordinary artwork and header-code compatibility.
- Corrected oversized save-file handling and stopped loading before changing cartridge save memory if closing the input file fails.

### Earlier features

- Brought the current DSSMM frontend to EZ-FLASH's published BLUE hardware foundation, with DS Style menus, themes, artwork and navigation.
- Added name-matched artwork for supported systems beyond GBA, mirrored artwork folders, long filenames and marked-thumbnail transparency. Existing opaque and GBA header-code artwork remain supported.
- Included the highly optimized artwork lookup and menu rendering paths while retaining transparency compatibility, including cached and scaled carousel previews, Start-screen artwork, favorites and recent games.
- Added optional **Remember cheats**, off by default, with per-game selections and localized labels. This builds on **exploitz86's** work; collapsible groups originate with **SkillerCMP**, adapted by **exploitz86**. The underlying game-time cheat engine remains BLUE's vendor implementation.
- Added DS Style NOR progress with a dedicated background, username, clock, percentage and progress bar. Installed NOR games can remember their original artwork folder through SD-side records.
- Preserved BLUE's native 64 MiB/5G handling, hardware drivers, FPGA firmware and NOR allocation format.

- **DS Style foundation (FrankieT19):** animated carousels, launch presentation and the separately authored v8.0 Start-screen/interface refinements, Auto theme, View grouping, Open SYSTEM, boot preferences and favorites behavior.
- List, List + Art, horizontal and vertical carousel views, title/box artwork and configurable side previews.
- Favorites, recently played games, last-played startup and configurable Start-screen behavior.
- Light, Dark and clock-based Auto themes, accent colors, menu sounds and settings stored in `SYSTEM/SETTINGS.TXT`.
- Multiple languages, including Chinese and Thai, plus the text/Markdown viewer.
- BLUE's native game loading, SRAM/save handling, in-game options and NOR library remain the hardware foundation.


### Earlier fixes

- Added checks for oversized or incomplete game/plugin files, combined emulator/game limits, failed SD reads and close errors. Failed loads stop before launching a partial image and rebuild directory records when loading has overwritten them.
- Added bounded plugin boot handling, including checks for compressed payloads and a reset handoff that protects the stack while clearing memory. Small valid plugin inputs no longer require a GBA header.
- Made SD save and settings replacement safer, with staged writes, retained previous complete files and tighter filename checks.
- Hardened NOR input, allocation and catalog handling, consistently padded writing blocks, and added erase/program/metadata readback checks. New allocations are reserved before catalog publication.
- Bounded patch copies to their working block and split PSRAM patch writes at memory-bank boundaries.
- Tightened cheat-file size checks and preserved collapsed-group row/selection fixes contributed by **exploitz86**.
- Removed the brief black progress strip during external-plugin launches for a cleaner themed transition.
- Checked kernel staging copies and BLUE firmware identity before replacing an update file.


### Test-build limitations

- **No physical BLUE validation.** Software tests and mocked BIOS/flash behavior cannot qualify installation, boot, performance, game compatibility or saving on a cartridge. Larger-image updater support is unknown.
- The vendor flash polling loops remain without a timeout. NOR catalog/allocation metadata are single-copy and are not power-loss atomic; readback checks do not change that limitation.
- Plugin compatibility depends on the plugin. Generic plugin ROM combinations remain limited to 32 MiB; BLUE's native 64 MiB handling is not assumed to apply to them. Extra custom theme/color slots are not included.
- Unusual NES patch layouts, physical timing and runtime stack peaks remain unqualified. This is an experimental port, not a hardware-qualified release.


### Download and testing

Download **`ezbluekernel.bin`** below. This is a **kernel-only BLUE test build**; use a BLUE-compatible `SYSTEM` setup. Back up your SD card and saves before any experimental testing. The About screen identifies `8.3.1-blue-test2`. Do not substitute the raw development `.gba` file, which lacks the required packed vendor resources.

The packed kernel is **3,982,696 bytes**. Protected layout/resource checks, five packaging test groups, platform loading/NOR/boot tests, and 24 shared software/UI suites passed. These checks do not replace hardware testing. GitHub supplies source archives automatically.

SHA-256: `ce7b74449b208f466636aa2c090a4621b7f48a08626ccbcc986e256401973710`

Thanks to **FrankieT19** for the DS Style foundation and interface work; **Sterophonick** and the **EZ-FLASH team** for the kernel foundations and BLUE source; **exploitz86** and **SkillerCMP** for the cheat-menu contributions described above; **aidiadayo** for Thai language support; **Kekun** for the case-sensitive font include correction in the project lineage; **ChaN** for FatFs; and the other contributors credited in the project. Merkin Mix artwork and port work is maintained by **Merkin / voluptuary**. See [CREDITS.md](https://github.com/voluptuary/ezfode-dssmm-kernel/blob/v8.3.1-blue-test2/CREDITS.md) for provenance.

This project was developed with AI assistance.
