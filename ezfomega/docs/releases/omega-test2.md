# Changelog

## DS Style Merkin Mix 8.3.1 — Original OMEGA Test 2

> **ORIGINAL EZ-FLASH OMEGA ONLY — HARDWARE UNTESTED.**
>
> This continues the Original OMEGA Merkin Mix port. Source, build and host checks have passed, but it has **not been tested on an Original OMEGA cartridge**. Installation, booting, gameplay, saving, plugins, cheats/save states and NOR writing still need hardware verification.
>
> **Do not install on the OMEGA Definitive Edition Rev. A or Rev. B, the newer OMEGA Definitive Edition B / BLUE, or another cartridge model.** The separate Definitive Edition 8.3.1 release remains the stable release for Rev. A/B.

### New features and improvements

- Brought over Merkin Mix's highly optimized artwork lookup and menu rendering paths while retaining transparency compatibility. Cached carousel motion handles opaque and transparent artwork, and CUSTOM artwork lookup avoids repeatedly scanning the entire artwork list.
- Added reliable indexed file identities for long filenames, with readable display names and separate aliases for opening the correct game or folder. Header-code artwork can use a validated fast lookup, with the ordinary lookup retained as a fallback.
- Added **CUSTOM Only** rounded corners, with the artwork choice retained correctly across carousel motion, scaled List + Art previews and popup restoration.
- Added editable **Clean list** display and **On, Not SYSTEM** wrapping. Settings accept the existing localized/legacy spellings and retain the selected scope.
- Added optional per-extension artwork defaults, such as `CUSTOM/gg.bmp`, and full-size missing-art previews for supported games and plugins.
- Added SD-side artwork records for NOR games, preserving their original artwork folder and long filename without changing the cartridge catalog format.
- Updated the splash and help artwork to the current Merkin Mix assets.

### Fixes

- Corrected stale thumbnail metadata when changing views, so list workspace reuse cannot leave a carousel preview marked as valid.
- Corrected the bounds check used when installing carousel drawing code into video memory.
- Added checks for raw and compressed plugin payloads before copying them into working RAM, including incomplete or malformed compressed data. Corrected the compressed-plugin decoder selection and the plugin name passed in launch arguments.
- Failed PSRAM loads now rebuild directory records before the menu uses that memory again.
- File copies now reject oversized size values, incomplete reads/writes and close errors, and remove detected incomplete output files.
- Corrected a missing Thai language-table entry that shifted subsequent labels, and retained localized remembered-cheat labels. Added readable confirm/back hints to game-management and delete popups.

### Earlier features

- Merkin Mix filename-matched artwork for supported systems beyond GBA, mirrored folders and long-filename artwork support in `SYSTEM/IMGS` and `SYSTEM/IMGS2`.
- Marked-thumbnail transparency alongside ordinary opaque images and GBA header-code artwork, including Start-screen previews, favorites and recent games.
- Optional **Remember cheats**, off by default, with per-game selections and collapsible groups. This builds on **exploitz86's** remembered-cheat work and the group-menu foundation by **SkillerCMP**, adapted by **exploitz86**.
- DS Style NOR writing progress with a percentage/bar, dedicated background, username and clock.
- **Original OMEGA DS Style v8.0 foundation (FrankieT19):** DS menus, Start screen, themes, menu sounds, animated carousels, selection effects and launch presentation.
- List, List + Art, horizontal and vertical views; title and box artwork; configurable side previews; favorites, recent games and last-played startup.
- Light, Dark and clock-based Auto themes, accent colors and readable settings in `SYSTEM/SETTINGS.TXT`.
- Multiple languages, including Chinese and Thai, and the text/Markdown viewer.

### Earlier fixes

- Removed the brief black progress strip during external-plugin launches.
- Added checks for game/plugin size limits, incomplete SD reads and loading across PSRAM memory boundaries, stopping before a partial image launches.
- Made SD saves and settings replacement safer with staged writes and retention of the previous complete file when a write fails.
- Hardened NOR input files, available-space/catalog checks and game-list scanning, with consistent final-block padding.
- Corrected patch copying across memory boundaries, bounded staged NOR patch copies and checked reads while preparing patched games.
- Corrected cheat-group selection and visible rows when groups collapse, including fixes contributed by **exploitz86**.

### Test-build limitations

- No Original OMEGA hardware results exist. Software checks and mocked storage/bus behavior cannot establish physical compatibility, performance, flash success or recovery from power loss.
- Native PSRAM games and generic plugin/game combinations remain limited to 32 MiB. NOR deletion retains the newest-title restriction.
- Original NOR programming APIs do not report physical success, and their polling remains without a timeout. Cartridge metadata is not power-loss atomic. Artwork hints and cheat snapshots are best-effort metadata; ordinary save/settings replacement has separate safeguards.
- Extra custom theme/color slots remain excluded. Hardware-specific DE/BLUE features and protocols are not part of this port.
- Malformed-input edge cases remain under the next adversarial audit; this is an experimental test build.

### Download and testing

Download **`ezkernel.bin`** below and use an Original OMEGA-compatible `SYSTEM` setup. This is a **kernel-only test build**. Back up your SD card and saves before testing. The About screen identifies **`Merkin Mix 8.3.1-omega-test2`**.

The kernel is **2,049,144 bytes**, below the 2 MiB limit. The complete target verifier passed, including save/settings fault injection, real FatFs identity/lookup checks, 766,800 artwork comparisons with no mismatches, settings/cache tests, plugin bounds/malformed-input checks and linked BIOS-wrapper checks. These do not replace hardware testing. GitHub supplies source archives automatically.

SHA-256: `a256c3dce226b3d6abdf6f9b13790cf610dead2b2b42f19411cee1d0d3e5e9a8`

Thanks to **FrankieT19** for his Original OMEGA DS Style adaptation and interface work; **Sterophonick** and the **EZ-FLASH team** for the kernel foundations; **exploitz86** and **SkillerCMP** for the cheat-menu contributions described above; **aidiadayo** for Thai language support; **Kekun** for the case-sensitive font include correction in the lineage; **ChaN** for FatFs; and the other contributors credited in the project. Merkin Mix artwork and backport work is maintained by **Merkin / voluptuary**. See [CREDITS.md](https://github.com/voluptuary/ezfode-dssmm-kernel/blob/v8.3.1-omega-test2/CREDITS.md) for provenance.

This project was developed with AI assistance.
