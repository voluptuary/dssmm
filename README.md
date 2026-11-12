# DS Style Merkin Mix (DSSMM)

DSSMM is a Nintendo DS-inspired custom menu kernel for the EZ-FLASH OMEGA family,
maintained by [Merkin](https://github.com/voluptuary). Merkin Mix
builds on [FrankieT19's DS Style forks](https://github.com/FrankieT19), with
foundations from EZ-FLASH and Sterophonick's SimpleLight kernel. On your
handheld, it provides the cartridge menu for browsing the SD card, launching
games and managing the card's supported save and NOR features.

Merkin Mix's main additions are **artwork transparency and name-matched game
artwork**. This lets games from supported emulator and plugin systems have their
own artwork, extending beyond the original kernels' GBA-only artwork lookup.
It also brings highly optimized menu rendering, more browsing and customization
options, remembered cheat selections, and improvements to saving and cartridge
management. Available features depend on the card and its compatible plugins.

## Choose the kernel for your exact card

**There are three distinct kernels. They are not interchangeable. Check your
cartridge model before downloading or flashing an update.**

| Your cartridge | Kernel download | Update file | Status |
| --- | --- | --- | --- |
| EZ-FLASH OMEGA **Definitive Edition Rev. A or Rev. B** | [DSSMM 8.3.2](https://github.com/voluptuary/dssmm/releases/tag/v8.3.2) | `ezkernelnew.bin` | Stable release |
| **Original EZ-FLASH OMEGA** | [DSSMM 8.3.1 — OMEGA Test 3](https://github.com/voluptuary/dssmm/releases/tag/v8.3.1-omega-test3) | `ezkernel.bin` | Experimental; not hardware-verified |
| Newer EZ-FLASH OMEGA **Definitive Edition B / BLUE** | [DSSMM 8.3.1 — BLUE Test 2](https://github.com/voluptuary/dssmm/releases/tag/v8.3.1-blue-test2) | `ezbluekernel.bin` | Experimental; not hardware-verified |

**The older Definitive Edition Rev. B and the newer Definitive Edition B / BLUE
are different cards.** The Rev. A/B release is also incompatible with the original
OMEGA. Use only the kernel and supporting SYSTEM files intended for your model,
and follow that release's installation instructions.

> [!WARNING]
> **The Original OMEGA and BLUE ports have not been tested on physical hardware.**
> Installation, booting, gameplay, saving, plugins, cheats/save states and NOR
> operations still need hardware verification. BLUE's larger update image has
> also not been verified with its updater. Software checks do not establish
> that these builds will work correctly on a cartridge.
>
> **Use both experimental builds with caution and only at your own risk.**
> Back up your SD card and save files before testing. Avoid powering off during
> kernel updates, save writes or NOR operations.

## GB/GBC emulator development update

Unreleased development code now uses [MiGoomba](https://github.com/michyprima/migoomba)
for GB/GBC games. The published downloads above still contain their previous
emulator. This integration needs hardware testing; back up `.esv` saves before
testing and do not assume old emulator save states are compatible.

## What DSSMM offers

- **Artwork beyond GBA:** name-matched images for supported systems, title or box
  artwork, mirrored artwork folders and fallback images. Existing GBA artwork
  lookup remains supported.
- **Transparent artwork:** compatible marked thumbnails can blend into the menu
  background. Menu rendering and artwork lookup are highly optimized while
  retaining transparency compatibility.
- **Several ways to browse:** list, List + Art, horizontal carousel and vertical
  carousel views, with favorites, recent games and last-played shortcuts.
- **A customizable DS-style menu:** Light, Dark and clock-based Auto themes,
  accent colors, menu sounds, artwork presentation, language and browsing options.
- **Remembered cheat choices:** optional per-game selections and collapsible
  cheat groups. In-game cheat and save-state capabilities depend on the card's
  own engine and the game or plugin being used.
- **Clearer cartridge management:** themed NOR writing progress, artwork for NOR
  library entries, and save/settings handling improvements adapted to each card.
- **More convenient navigation:** configurable startup behavior, wrapping,
  favorites shortcuts and access to the SYSTEM folder.

Hardware capabilities remain different across the three cards. A feature on one
card does not add that hardware capability to another, and plugin compatibility
still depends on the plugin. Flash operations and interrupted save or metadata
writes retain known limitations, so keep backups even when using a stable build.

For platform-specific features, artwork requirements and known limitations, see
the [Definitive Edition Rev. A/B guide](ezfomegade/README.md),
[Original OMEGA guide](ezfomega/README.md) or [BLUE guide](ezfblue/README.md).
[DS Style Card Companion](https://github.com/voluptuary/dsscc) is the separate
companion application for finding and preparing artwork.

## Credits

DSSMM builds on the work of the following contributors:

- **Merkin** — Merkin Mix development and maintenance, artwork and
  transparency support, refreshed splash screens and system icons, rendering
  optimizations, kernel fixes and the platform ports, with AI assistance.
- **FrankieT19** — the DS Style foundations, animated carousels and launch
  presentation, separately authored v8.0 interface and navigation refinements,
  and the Original OMEGA DS Style adaptation used as that port's foundation.
- **The EZ-FLASH team**, including **fengflash** and **Windirt Young** — the
  original kernel sources and hardware foundations, including the BLUE source.
- **Sterophonick** — SimpleLight, part of the custom-kernel foundation.
- **exploitz86** — remembered per-game cheat selections, cheat-menu row and
  collapsed-group handling, and localized setting labels.
- **SkillerCMP** — the collapsible cheat-group foundation and cheat, save-state
  and patch-system contributions in the upstream lineage.
- **aidiadayo** — Thai language support.
- **Kekun** — the font-header capitalization correction used for compatibility
  with case-sensitive build systems.
- **ChaN** — FatFs, the filesystem library used by the kernels.
- **michyprima** — MiGoomba, the GB/GBC emulator in development builds, based on
  **Jaga / EvilJagaGenius**'s Jagoomba Color, **Dwedit**'s Goomba Color and
  **FluBBa**'s Goomba. Its upstream acknowledgments also credit **Minucce**,
  **Nuvie**, **Radimerry**, **Therealteamplayer** and **Endrift / mGBA**.
- **Copilot, ChatGPT, and Claude** — contributions to interface integration, testing and
  release documentation. Other AI-assisted development work
  are recorded in the project history.

Thank you also to the upstream component authors, artwork/tool contributors,
and everyone who has tested builds and reported problems. The platform
[credit records](CREDITS.md) retain detailed attribution and provenance.

The repository includes the [Apache License 2.0](LICENSE). Individual components
retain their own licenses and attribution notices.
