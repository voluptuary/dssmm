# DS Style Merkin Mix

DS Style Merkin Mix is a custom kernel for the **EZ-FLASH OMEGA Definitive Edition**, maintained by [Merkin](https://github.com/voluptuary). This Merkin Mix fork builds on [FrankieT19's DS Style fork](https://github.com/FrankieT19/omega-de-ds-style-kernel). Its main addition is name-matched game artwork with selective transparency, allowing artwork for games on all systems supported by the kernel rather than only the GBA game artwork in the original kernels. Merkin Mix also contributes fixes and enhancements to the cheat system and updates to the cartridge's SD and NOR workflows and management.

> [!WARNING]
> **Supported hardware:** EZ-FLASH OMEGA Definitive Edition rev. A and rev. B only.
>
> **Not supported:** the newer, similarly named EZ-FLASH OMEGA Definitive Edition B or the original EZ-FLASH OMEGA.

## Download and install

Get the stable [DS Style Merkin Mix 8.4.0 release](https://github.com/voluptuary/dssmm/releases/tag/v8.4.0). Copy `ezkernelnew.bin` to the root of the SD card, then start the cartridge while holding **R** to install the kernel. Back up saves before updating.

## What it offers

- **Choose how to browse:** use list, List + Art, horizontal carousel or vertical carousel views, with favorites and recently played games.
- **Make it yours:** choose themes, artwork style, borders, rounded corners, language, clock format and menu behavior. Auto theme changes between light and dark at set times.
- **Find and view artwork:** use title or box images, mirrored and custom artwork, built-in fallbacks, and artwork for games in nested SD or NOR folders. Menu artwork rendering is highly optimized while marked transparent artwork remains supported.
- **Get to games quickly:** use saved favorites, optional wrap-around, Boot to preferences, and double-tap **Select** to change a favorite.
- **Keep cheat choices:** optional Remember cheats saves selections per game; collapsing cheat groups retains the selected choices.
- **Manage the cartridge:** browse the SYSTEM folder from the menu, install supported kernels from `/SYSTEM/KERNELS`, and receive clearer feedback during NOR operations.

## Known issues

Known remaining limits include flash operations without a chip-qualified timeout, NOR catalog updates that are not power-loss atomic, and SD-file replacements that can require manual recovery after an interruption. Back up saves regularly and avoid powering off during writes. See the [audit report](docs/audits/cross-platform-2026-11.md) for validation limits.

## Custom artwork

Transparent thumbnails require the `DST1` marker in BMP header bytes 6–9. Use an uncompressed, top-down 16-bit BMP at **120 × 80** for title art or **80 × 80** for box art. In marked images, RGB555 `0x0000` is transparent; use a nonzero near-black value for visible black. Unmarked thumbnails are opaque.

[DS Style Card Companion](https://github.com/voluptuary/dsscc) is a separate artwork and scraper companion.

## Credits and license

Merkin Mix is maintained by Merkin and builds on FrankieT19's DS Style v7.5 foundation and selected, separately authored v8.0 interface work. Merkin contributed the refreshed splash screens and system icons in the v8.0 lineage, along with Merkin Mix artwork and kernel improvements. EZ-FLASH, Sterophonick, Copilot, exploitz86 and SkillerCMP also contributed to the project's foundations and features; [CREDITS.md](CREDITS.md) records the specific provenance.

The repository includes the [Apache License 2.0](../LICENSE). Retain individual component licenses and attribution notices.

## GB/GBC emulator

This release bundles [MiGoomba](https://github.com/michyprima/migoomba)
for Game Boy and Game Boy Color games. This replacement has source/build/software testing,
but needs hardware testing in DSSMM. Back up `.esv` saves before testing; old
emulator save states are not guaranteed compatible.
