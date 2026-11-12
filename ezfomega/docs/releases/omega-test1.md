# Changelog

## DS Style Merkin Mix 8.3.1 — Original OMEGA Test 1

> **ORIGINAL EZ-FLASH OMEGA ONLY — HARDWARE UNTESTED.**
>
> This is the first public test of the Original Omega Merkin Mix port. It has passed source, build, and host-side checks, but has **not been tested on an original Omega cartridge**. Booting, gameplay, saving, sleep/save states, and NOR writing still need hardware verification.
>
> **Do not install on the OMEGA Definitive Edition Rev. A or Rev. B, the newer OMEGA Definitive Edition B/BLUE, or any other cartridge model.** The separate Definitive Edition 8.3.1 release remains the stable release for that hardware.

### New features and improvements

- Brought Merkin Mix artwork support to FrankieT19's Original Omega DS Style v8.0 foundation. Filename-matched artwork allows previews for supported systems beyond GBA, with mirrored folders and long-filename support in `SYSTEM/IMGS` and `SYSTEM/IMGS2`.
- Added marked-thumbnail transparency while retaining ordinary opaque artwork and existing GBA header-code artwork. Filename artwork is also connected to carousel caches, Start-screen previews, favorites, and recently played games.
- Added optional **Remember cheats**, off by default, with localized labels and per-game selections. This builds on **exploitz86's** work. Collapsible cheat groups are based on **SkillerCMP's** work, adapted by **exploitz86**.
- Gave NOR game writing a DS Style progress window with a percentage and progress bar, a dedicated background, and the usual username and clock on the top bar.

### Fixes

- Removed the brief black progress strip during external-plugin launches for a cleaner themed transition.
- Added checks for oversized games and emulator/game combinations, incomplete SD reads, and loading across PSRAM memory boundaries. Failed loads stop before launching a partial image.
- Made save and settings writes safer by staging replacement files and retaining the previous complete file when an SD write fails.
- Added checks for NOR input files, available space, catalog limits, and incomplete reads. Unused bytes in the final writing block are padded consistently.
- Hardened NOR game-list scanning against invalid entries and full catalogs.
- Corrected patch copying across memory boundaries and bounded NOR patch copies to their working buffer. Added checks for short or failed reads while preparing patched games.
- Corrected cheat-group selection and visible-row handling when groups are collapsed, including fixes contributed by **exploitz86**.

### Earlier features

- **Original Omega DS Style v8.0 foundation (FrankieT19):** DS-style launcher, Start screen, menu sounds, themes, selection effects, animated carousels, and launch presentation.
- List, List + Art, horizontal carousel, and vertical carousel views; title and box artwork; configurable side previews; favorites and recently played games.
- Light, Dark, and clock-based Auto themes, multiple accent colors, customizable launcher assets, and readable settings in `SYSTEM/SETTINGS.TXT`.
- Multi-language launcher text, including Chinese and Thai, and a text/Markdown viewer.
- Existing Original Omega game loading, cheats, save handling, and NOR support are retained as the hardware foundation of this port.

### Test-build limitations

- This is a partial backport, not full feature parity with the Definitive Edition build. Additional fast artwork lookup/rendering paths and some settings behavior remain to be ported.
- NOR artwork can use the installed catalog name and root/header fallbacks. Remembering the original installation folder for nested NOR artwork is not implemented yet.
- Software checks cannot establish physical flash-programming success or guarantee recovery from power loss. Cheat-selection snapshots also do not have the same replacement safeguards as game saves.
- The original cartridge's kernel-size restrictions still apply; separate extra custom theme/color slots are not included.

### Download and testing

Download **`ezkernel.bin`** below. This is a **kernel-only test build**; use an Original Omega-compatible `SYSTEM` setup. Back up your SD card and saves before testing. The About screen identifies this development checkpoint as `Merkin Mix 8.3.1-omega-dev`.

The kernel is 2,044,264 bytes, below the 2 MiB limit. Source, build, and host-side tests passed, including artwork rendering comparisons, save/settings fault injection, and mocked loading/NOR tests. These do not replace hardware testing.

SHA-256: `bb7a5302d7711b4340d595856e2e5eaf433bc6cad45c20fb0850f5dfc678c336`

Thanks to **FrankieT19** for his Original Omega DS Style adaptation and interface work; **Sterophonick** and the **EZ-FLASH team** for the kernel foundation; **exploitz86** and **SkillerCMP** for the cheat-menu contributions described above; **aidiadayo** for Thai language support; and the other contributors credited in the project. Merkin Mix artwork and backport work is maintained by **Merkin / voluptuary**. See [CREDITS.md](https://github.com/voluptuary/dssmm/blob/v8.3.1-omega-test1/CREDITS.md) for provenance.

This project was developed with AI assistance.
