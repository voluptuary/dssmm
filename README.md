# DS Style Merkin Mix

DS Style Merkin Mix is a custom kernel for the **EZ-FLASH OMEGA Definitive Edition**, maintained by [Merkin](https://github.com/voluptuary). It brings together a flexible game browser, configurable artwork, saved favourites and cheat selections, and practical updates to the cartridge's SD and NOR workflows.

> [!WARNING]
> **For OMEGA Definitive Edition only.** Do not install this kernel on the original EZ-FLASH OMEGA.

## Download and install

Get the stable [DS Style Merkin Mix 8.3.0 release](https://github.com/voluptuary/ezfode-dssmm-kernel/releases/tag/v8.3.0). Copy `ezkernelnew.bin` to the root of the SD card, then start the cartridge while holding **R** to install the kernel. Back up saves before updating.

## What it offers

- **Choose how to browse:** use list, List + Art, horizontal carousel or vertical carousel views, with favourites and recently played games.
- **Make it yours:** choose themes, artwork style, borders, rounded corners, language, clock format and menu behavior. Auto theme changes between light and dark at set times.
- **Find and view artwork:** use title or box images, mirrored and custom artwork, built-in fallbacks, and artwork for games in nested SD or NOR folders. Menu artwork rendering is highly optimized while marked transparent artwork remains supported.
- **Get to games quickly:** use saved favourites, optional wrap-around, Boot to preferences, and double-tap **Select** to change a favourite.
- **Keep cheat choices:** optional Remember cheats saves selections per game; collapsing cheat groups retains the selected choices.
- **Manage the cartridge:** browse the SYSTEM folder from the menu, install supported kernels from `/SYSTEM/KERNELS`, and receive clearer feedback during NOR operations.

### Changes in 8.3.0

- Improved artwork responsiveness, including faster header-art lookup in large game folders, while preserving marked-thumbnail transparency.
- Refined the Start screen, launch feedback, menu sounds and pop-ups; added Auto theme, Boot to controls and a shortcut for changing favourites.
- Improved artwork and status feedback in the startup save-recovery screen, and made NOR operation errors clearer.

## Known issues

The next maintenance focus is safer recovery when replacing an existing save, clearer reporting of settings-file write failures, and more resilient NOR catalog updates if an operation is interrupted. Back up saves regularly and avoid powering off during NOR writes.

## Custom artwork

Transparent thumbnails require the `DST1` marker in BMP header bytes 6–9. Use an uncompressed, top-down 16-bit BMP at **120 × 80** for title art or **80 × 80** for box art. In marked images, RGB555 `0x0000` is transparent; use a nonzero near-black value for visible black. Unmarked thumbnails are opaque.

[DS Style Card Companion](https://github.com/voluptuary/dsscc) is a separate artwork and scraper companion.

## Build from source

Build with devkitPro's **devkitARM, GBA rules, libgba, Maxmod and Make**. The checked-in devcontainer uses `devkitpro/devkitarm:20260610`.

From the repository root with the devkitPro environment loaded:

```sh
make -B
```

The output `.gba` file uses the checkout directory's name. The Makefile enforces the **2 MiB** kernel limit. Generated theme headers are checked in; after changing theme images, regenerate them with `Grit\Build Skin Files.bat`.

For the build and regression checks, install the script's host dependencies and run:

```sh
bash scripts/rebuild-and-verify-audit.sh
```

## Credits and license

Merkin Mix is maintained by Merkin. It includes FrankieT19's DS Style v7.5 foundation and selected, separately authored v8.0 interface work. FrankieT19 is not credited for changes that originated in Merkin Mix. EZ-FLASH, Sterophonick, Copilot, exploitz86 and SkillerCMP also contributed to the project's foundations and features; [CREDITS.md](CREDITS.md) records the specific provenance.

The repository includes the [Apache License 2.0](LICENSE). Retain individual component licenses and attribution notices.
