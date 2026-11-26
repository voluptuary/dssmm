**SUPERCARD SD, SUPERCARD LITE AND SUPERCHIS ONLY.** Choose the firmware for your exact card.

Only Chis has been tested on hardware, on SuperChis Prime. SD and Lite remain physically untested. Back up saves and firmware before updating.

Build 822 adds reference-informed cartridge icons, settings rows aligned with the native EZ-FLASH DE menu, and a corrected native PCE return path. The experimental Atari 2600 plugin remains a separate external artifact; it is not bundled in this firmware.

### Features added since the SuperFW fork

- **DS Style Start screen** with last-played artwork, SD/NOR shortcuts, a username from `SYSTEM/NAME.TXT` and an editable 12/24-hour menu clock. B at the SD root returns to Start.
- List, horizontal and vertical artwork browsing with a **five-image cache**, optional carousel wraparound and Clean list to hide NoIntro tags in displayed titles.
- **Title and Box artwork:** CUSTOM overrides, filename-matched DSSCC packs, directory pictures and global `#`/A–Z folder art, including marked transparency. Title mode also supports original EZ-FLASH GBA ROM-header artwork.
- **Chis NOR carousels and artwork**, including stored-code Title art and Last Played pictures without the original SD ROM. List mode retains NOR capacity and management controls.
- DS Style backgrounds and framed menus, grouped settings, an optional DSSMM font, Light/Dark themes, selection accents and independent shaded top-bar colors.
- **Settings layout** now follows the native EZ-FLASH DE row spacing and highlight baseline.
- A shared **SYSTEM layout** using `SETTINGS.TXT`, `RECENT.TXT`, `SAVER` and `PLUG`. Settings save automatically with verification and recovery; legacy folders and saves migrate without overwriting conflicts.
- Built-in **MiGoomba v0.1** GB/GBC emulation on all three variants, alongside optional external emulator overrides and Chis’s other bundled emulators.
- Portable **Pogoshell BIN/GBA/MB/MBZ plugins**, including shared SMS/Game Gear plugin selection and supported emulator-return handling.
- **Cartridge icons** for Game Boy/Color, Game Gear, PC Engine/HuCard, Neo Geo Pocket/Color and Atari 2600 files, packed read-only within the existing palette.
- Chis GBA launch screens offer **Load or Write to NOR**, with capacity checks, confirmation and replacement prompts. Framed progress screens describe loading and flashing operations.
- Firmware-update unlock with **Down, Up, Left, Right within five seconds**. Compressed menu resources recover SD/Lite firmware space within the original hardware limits.
- **Atari 2600 support remains experimental and external.** The firmware recognises `.a26` files and shows the cartridge icon, but no Atari emulator is bundled. The plugin is loaded from `SYSTEM/PLUG`; development continues at [a26.gba](https://github.com/voluptuary/a26.gba).

### Fixes to upstream SuperFW

- Corrected maximum-length parent paths and unsigned-byte filename comparisons; guarded the SD benchmark against zero elapsed time.
- Bounded patch scanning and operation counts, corrected patch insertion ordering and retained signatures across scan-window boundaries.
- Hardened native GBA/NOR loading with allocation and payload-space checks, exact reads, aligned odd-size tails and cleanup after failed transfers.
- Staged native emulator headers in RAM and corrected partial-block transfers, file-size checks and close-error handling.
- Corrected the native PCE plugin return path; the owner reports that Exit works in Build 822. Preserved Lite stack headroom by keeping sprite command storage in EWRAM.
- Hardened the experimental external Atari runtime with RIOT timer/WSYNC corrections, bounded raster queues and off-screen sprite scratch guards.
- Corrected optional NDS-driver handling and file cleanup. These checks do not establish DS hardware compatibility.

### Known issues and hardware testing

- **Only Chis has hardware testing.** Builds, ARM replay and mGBA checks do not qualify SD/Lite operation or physical flash/save timing.
- The owner reports Atari games load and Exit through the external plugin on Chis Build 822; the exact plugin revision was not stated. PCE Exit now works in the owner’s Build 822 test. Other plugins may still depend on unsupported card behavior.
- Atari 2600 reimplementation is incomplete: audio is not implemented, only selected 2 KiB/4 KiB/F8/F6 mappings are accepted, and compatibility/performance are not qualified. The external plugin is not part of this download.
- Full EZ-FLASH DSSMM appearance and responsiveness remain in progress. Some Start artwork sizing and fallback icons still differ; SD operations can briefly stall a frame. Compressed-resource startup on SD/Lite needs hardware testing.
- Old emulator save states are not guaranteed compatible; prefer in-game battery saves when changing emulators. The menu clock does not advance while powered off.
- Confirming NOR replacement removes existing games; a later write failure cannot restore them.

### SD-card setup

1. Download the `.fw` for **Chis, SD or Lite** as appropriate. Back up saves and follow [SuperFW’s installation and recovery guide](https://superfw.davidgf.net/docs/install/flash/).
2. Keep games in your preferred folders. The kernel uses `SYSTEM/SETTINGS.TXT`, `SYSTEM/RECENT.TXT` and `SYSTEM/SAVER`. An optional username goes on the first line of `SYSTEM/NAME.TXT`.
3. Extract the attached **folder-art.zip** to the SD root, merging its `SYSTEM` folder. Title art uses `SYSTEM/IMGS`; Box art uses `SYSTEM/IMGS2`; overrides go in each folder’s `CUSTOM` directory. See [artwork setup](https://github.com/voluptuary/dssmm/blob/superfw/superfw/docs/ARTWORK-SETUP.md) for DSSCC filename packs and ROM-header art.
4. Optional plugins go in `SYSTEM/PLUG`; see [plugin setup](https://github.com/voluptuary/dssmm/blob/superfw/superfw/docs/PLUGIN-COMPATIBILITY.md). Atari 2600 files need the separate [a26.gba](https://github.com/voluptuary/a26.gba) plugin. Select artwork, font and colors under Settings → Interface, and let settings saves finish before powering off.

### Credits

Thanks to **[davidgfnet](https://github.com/davidgfnet/superfw)** for SuperFW, **[FrankieT19](https://github.com/FrankieT19)** for DS Style, and **michyprima**, **Jaga / EvilJagaGenius**, **Dwedit**, **FluBBa** and the other emulator authors. Atari 2600 prototype work builds on **MikeDX/2600_gba** and its credited Virtual VCS/X2600, Daniel Boris, Alex Hornby and x64 team components. Component credits and license notices remain in the source.
