## New features

- DS Style Start screen with last-played artwork, a username from `SYSTEM/NAME.TXT`, SD/NOR shortcuts and an editable 12/24-hour menu clock.
- Horizontal and vertical artwork carousels with five-image lookahead, list browsing, and Clean list to hide NoIntro tags in displayed titles.
- Title and Box artwork using DSSCC filename packs, CUSTOM overrides, directory pictures and global `#`/A–Z folder art. Title mode also supports EZ-FLASH GBA ROM-header artwork.
- DS Style backgrounds and framed menus, grouped settings, an optional DSSMM font, Light/Dark themes, selection accents and independent top-bar colors.
- A shared `SYSTEM` layout with portable `SETTINGS.TXT`, `RECENT.TXT`, `SAVER` saves and `PLUG` emulator plugins. Legacy folders/saves migrate without overwriting conflicts. Settings save automatically in background steps, with verification and recovery.
- Portable Pogoshell BIN/GBA/MB/MBZ plugin loading alongside SuperFW’s bundled emulator fallbacks. Game Gear uses the SMS plugin and its own cartridge icon.
- Chis ROM screens offer Load or Write to NOR, with capacity checks, confirmation and replacement prompts. B at the SD root returns to Start; firmware updating unlocks with Down, Up, Left, Right within five seconds.

## Fixes to upstream SuperFW

- Corrected maximum-length parent-path handling and unsigned-byte string comparisons; guarded the SD benchmark against zero elapsed time.
- Bounded native patch scanning and operation counts, corrected patch insertion ordering, and preserved signatures across scan-window boundaries.
- Hardened native GBA/NOR loading with allocation and payload-space checks, exact reads, odd-size tails and cleanup after failed transfers.
- Fixed optional-driver handling and file cleanup in the NDS loader. These checks do not establish DS hardware compatibility.

## Known issues and hardware testing

- **Only the Chis version has been tested on hardware**, on a SuperChis Prime. SD and Lite remain physically untested. Build 667’s new changes still need hardware testing; builds, ARM replay and mGBA checks do not qualify physical flash/save timing or gameplay.
- GG/SMS emulator Exit can crash or leave a black screen. PCE loading and other Pogoshell plugin compatibility remain incomplete; some plugins depend on unsupported card-specific behavior.
- Full EZ-FLASH DSSMM appearance and performance parity is still in progress. Individual SD operations can briefly stall a frame; very large settings files save synchronously.
- The software menu clock does not advance while powered off. Confirming NOR replacement removes existing games; a later write failure cannot restore them.

## SD-card setup

1. Download the `.fw` for your exact card: Chis, SD or Lite. Back up saves and firmware, then follow [SuperFW’s installation and recovery guide](https://superfw.davidgf.net/docs/install/flash/) to load or install it. These builds are for SuperCard-family hardware.
2. Keep games in folders of your choice. The kernel uses `SYSTEM/SETTINGS.TXT`, `SYSTEM/RECENT.TXT` and `SYSTEM/SAVER`. Optionally put your username on the first line of `SYSTEM/NAME.TXT`.
3. Extract the attached **folder-art.zip** to the SD root, merging its `SYSTEM` folder. Title art uses `SYSTEM/IMGS`; Box art uses `SYSTEM/IMGS2`; overrides go in each `CUSTOM` subfolder. See [artwork setup](https://github.com/voluptuary/dssmm/blob/dssmm/superfw/docs/ARTWORK-SETUP.md) for filename packs and ROM-header art.
4. Optional plugins go in `SYSTEM/PLUG`; see [plugin setup](https://github.com/voluptuary/dssmm/blob/dssmm/superfw/docs/PLUGIN-COMPATIBILITY.md). Choose artwork, font, clock and colors under Settings → Interface. Allow background saves to finish before powering off.

## Credits

- [davidgfnet](https://github.com/davidgfnet/superfw) for SuperFW.
- [FrankieT19](https://github.com/FrankieT19) for DS Style.
- The emulator authors for their emulators and plugins. Component license notices remain in the source.
