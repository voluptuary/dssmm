# DS Style Merkin Mix — Original EZ-FLASH OMEGA

Merkin Mix adds artwork transparency and name-matched artwork for supported systems beyond GBA, alongside highly optimized artwork lookup and menu rendering that retains transparency compatibility. This branch builds on **FrankieT19's Original OMEGA DS Style v8.0 fork**, with the EZ-FLASH and Sterophonick kernel foundations.

**Original EZ-FLASH OMEGA only. Hardware untested.** These builds are incompatible with the older OMEGA Definitive Edition Rev. A/B and the newer OMEGA Definitive Edition B / BLUE. Those targets have separate [ezfode](https://github.com/voluptuary/ezfode-dssmm-kernel/tree/ezfode) and [ezfblue](https://github.com/voluptuary/ezfode-dssmm-kernel/tree/ezfblue) branches.

The current development checkpoint identifies itself as `Merkin Mix 8.3.1-omega-test3`. It has source, build and host-test validation only. Installation, booting, gameplay, saving, plugins, cheats/save states and NOR operations still need testing on an Original OMEGA cartridge. [Original OMEGA Test 3](https://github.com/voluptuary/ezfode-dssmm-kernel/releases/tag/v8.3.1-omega-test3) is the current hardware-untested prerelease.

Features include:

- Name-matched and mirrored artwork folders, long-filename support, marked transparency, GBA header-code artwork and per-extension default images.
- List, List + Art, horizontal and vertical carousels, title/box artwork, cached previews and optional CUSTOM Only rounded corners.
- Favorites, recent and last-played games, configurable Start-screen behavior, Light/Dark/Auto themes, accent colors and menu sounds.
- Clean list display, configurable wrapping including “On, Not SYSTEM,” multiple languages including Chinese and Thai, and the text/Markdown viewer.
- Optional remembered per-game cheat selections and collapsible groups, adapted from **exploitz86's** work and **SkillerCMP's** group-menu foundation.
- Staged SD save/settings replacement, checked loading and patch preparation, themed NOR writing progress with the normal clock/username, and remembered NOR artwork source paths on SD.

The Original OMEGA hardware drivers and game-time cheat engine remain the target foundation. Native PSRAM loading and generic plugin combinations are limited to 32 MiB. NOR deletes retain the newest-title restriction. NOR flash status and power-loss recovery remain limited by the original implementation; SD artwork hints and cheat snapshots are best-effort metadata. Extra custom theme/color slots remain excluded.

Download the Original OMEGA **`ezkernel.bin`** from its explicitly labeled prerelease and use an Original OMEGA-compatible `SYSTEM` setup. Back up your SD card and saves before testing. GitHub provides source archives automatically.

Build with devkitPro/devkitARM and libgba:

```sh
make BUILD=build-omega TARGET=omega-port
bash scripts/verify-omega-port.sh
```

The verifier requires a host C compiler, Python 3 and the devkitPro tools. The build enforces the 2 MiB kernel limit. Host tests check filesystem failures, artwork rendering, settings, caches, directory identities, loader policies and patch boundaries; they do not qualify physical hardware. See [port status and validation](docs/porting/OMEGA_STATUS.md).

Thanks to **FrankieT19** for the Original OMEGA DS Style adaptation and interface work, **Sterophonick** and **EZ-FLASH** for the foundations, **exploitz86** and **SkillerCMP** for the cheat-menu contributions described above, **aidiadayo** for Thai support, **Kekun** for the case-sensitive font include correction in the lineage, and **ChaN** for FatFs. Merkin Mix artwork and port work is maintained by **Merkin / voluptuary**, with AI assistance. [Detailed credits and provenance](CREDITS.md).
