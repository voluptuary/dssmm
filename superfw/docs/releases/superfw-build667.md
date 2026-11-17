## New features

- **Top bar color** in Settings → Interface is independent of Accent. Default restores gray with black text; Blue, Green, Orange and Purple use white text.
- **Clock format** offers the default 24-hour display or 12-hour time with AM/PM.
- Settings save in small background steps so you can continue navigating. Further edits are queued, unchanged settings skip the write, and pending saves finish before launching a game.

## New fixes

- Fixed the Interface-settings freeze that caused Test 6 to be withdrawn. Saves retain read-back verification, rollback on failure and recovery after an interrupted replacement.
- Aligned the clocks across menus, centered horizontal carousel titles, moved the SuperFW logo down and removed empty settings headings. Stock Dark is now Gray; Recents uses the NOR-style list background.
- Loading/Patch options use themed controls, and ROM Load/Write uses the accent highlight. Both fonts keep scrolling help and selection ink inside panel frames; launch panels clear carousel remnants.
- Refined the Game Gear cartridge: one pixel shorter, with a smaller illustrated label and maroon left strip.

## Summary of past features

- **DS Style menus:** Start screen with last-played artwork, username from `SYSTEM/NAME.TXT`, editable menu clock, SD/NOR shortcuts and grouped Interface/Games/Hardware/System/Help pages. Includes Light/Dark themes, accents and the optional DSSMM font.
- **Artwork browsing:** horizontal/vertical carousels with five-image lookahead, list views, Title/Box artwork and Clean list to hide NoIntro tags. CUSTOM overrides precede filename pictures; Title then checks GBA ROM-header artwork. Supports DSSCC packs, custom directory images and global `#`/A–Z folder art.
- **Portable SD layout:** one `SYSTEM/SETTINGS.TXT` with shared EZ-FLASH settings, `RECENT.TXT` history, `SAVER` saves and `PLUG` emulator plugins. Legacy directories/saves migrate without overwriting conflicts. Game Gear shares the SMS plugin; portable BIN/GBA/MB/MBZ plugins precede bundled fallbacks.
- **Native SuperFW functions:** GBA loading, cheats, in-game menus and bundled emulators remain across Chis, SD and Lite. Chis ROM screens offer Load or Write to NOR, with capacity checks, confirmation and replacement prompts. MiGoomba is not included.

## Summary of past fixes

- Fixed browser wraparound crashes, cartridge-memory artwork writes, EZ-FLASH ROM-header Title formats and DSSCC transparency; reduced artwork reads/scaling/redraws and removed the experimental Animation switch.
- Preserved two-character GB/GG save names and legacy saves; consolidated settings/history, retained artwork Off and protected unknown settings. Hardened ROM/NOR allocation, reads, odd-size tails, save paths, patch scanning and failed-load cleanup.
- Added lined dialog panels, refined Start selectors and centered System options. B at the SD root returns to Start; firmware updating unlocks with Down, Up, Left, Right within five seconds.

**SD-card setup**

1. Download the `.fw` for your exact card: `superfw-chis.fw`, `superfw-sd.fw` or `superfw-lite.fw`. Back up saves and firmware, then follow [SuperFW’s installation/recovery guide](https://superfw.davidgf.net/docs/install/flash/) for loading or installing it. These files are for SuperCard-family hardware, not EZ-FLASH cards.
2. Keep games in folders of your choice. The kernel creates/uses `SYSTEM`; settings are stored in `SYSTEM/SETTINGS.TXT`, saves in `SYSTEM/SAVER`, and last-played history in `SYSTEM/RECENT.TXT`. Optionally put your username on the first line of `SYSTEM/NAME.TXT`.
3. Extract the attached **folder-art.zip** to the SD root and merge its `SYSTEM` folder, keeping your own pictures if preferred. Title art belongs in `SYSTEM/IMGS`; Box art in `SYSTEM/IMGS2`. Put overrides in their `CUSTOM` subfolders. [Artwork setup](https://github.com/voluptuary/dssmm/blob/dssmm/superfw/docs/ARTWORK-SETUP.md) explains DSSCC filename packs and ROM-header art.
4. Optional emulator plugins go in `SYSTEM/PLUG`; see [plugin setup](https://github.com/voluptuary/dssmm/blob/dssmm/superfw/docs/PLUGIN-COMPATIBILITY.md). Use Settings → Interface for font, artwork, browser, clock and colors. Allow background saves to finish before powering off.

**Known limits:** Only the **Chis version has been tested on hardware** by the owner; SD and Lite remain physically untested. Earlier Chis testing confirms artwork, five-image caching and settings persistence. The new changes pass three-board builds, host checks, actual ARM replay and mGBA menu/persistence tests, but physical flash/save timing and gameplay are not qualified by those checks. GG/SMS plugin Exit can crash; PCE loading and some plugin compatibility remain unresolved. Full EZ-FLASH DSSMM appearance/performance parity is still in progress. Individual SD operations can briefly stall a frame; near-capacity settings files use synchronous saves. The menu clock does not advance while powered off. Confirming NOR replacement removes existing games and cannot be undone after a later write failure.

Credits: [David Guillen Fandos/davidgfnet](https://github.com/davidgfnet/superfw) for SuperFW; ChisBread and collaborators for SuperChis; Merkin for the DSSMM port; DS Style/DSSMM contributors and emulator/plugin authors; DSSCC and its FrankieT19 DS Style scraper lineage for artwork tooling. Component license notices remain in the source.
