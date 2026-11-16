## New features

- Chis GBA launch screens now offer **Load GBA game ROM** or **Write GBA ROM to NOR**. Use the D-pad to switch and A to continue. NOR writes require confirmation; when existing games leave too little space, you can confirm replacing them. ROMs above the 32 MiB per-game limit are rejected.

## New fixes

- Help, System, ROM launch and confirmation screens now share the DS Style lined panel; launch screens clear the carousel beneath them. Restored the graphical SuperFW logo and NOR list footer, centered System options vertically, and adjusted Start markers and horizontal title bounds. The Game Gear cartridge is shorter, wider and has a blank label.
- Only `SYSTEM/SETTINGS.TXT` supplies settings; obsolete UI/US sidecars are ignored. Changes save immediately and on exit, with read-back verification and preservation of the previous file if verification fails. Shared EZ-FLASH text settings remain supported.
- Raw `.mb` plugins accept correctly padded final words, matching their RAM-copy requirements. PCE compatibility still needs hardware confirmation.
- Faster artwork scaling with the same image placement and transparency; removed unused background data to free firmware space.
- Safer native ROM loading and NOR writes: checked memory allocation, complete reads, odd-size tails and cleanup. A failed load that has overwritten menu resources now requests a restart.
- Fixed maximum-length save paths, patch-scanner boundaries and ordering, and failed NDS-load cleanup. Recent-history read errors preserve the existing file; settings read failures now show an error and prevent overwriting the original settings.

## Summary of past features

- DS Style Start screen with last-played artwork, username, clock, SD/NOR shortcuts and grouped settings; optional DSSMM font, Clean list, Light/Dark themes and accents.
- Horizontal/vertical carousels with five-image lookahead; List/Title/Box artwork, CUSTOM folders and global `#`/A–Z images. Supports DSSCC filename packs and EZ-FLASH GBA game-code Title art.
- Shared `SYSTEM/SETTINGS.TXT`, automatic settings saves, `RECENT.TXT` history, `SAVER` saves and `PLUG` emulator plugins. BIN/GBA/MB/MBZ plugins precede native fallbacks; Game Gear shares the SMS plugin.
- Based on [David Guillen Fandos’s SuperFW](https://github.com/davidgfnet/superfw), retaining native loading, cheats, in-game menus and bundled emulators across Chis, SD and Lite. NOR remains Chis-specific; MiGoomba is not included.

## Summary of past fixes

- Fixed browser-edge crashes, cartridge-memory artwork writes, ROM-header Title compatibility and DSSCC transparency; CUSTOM and filename pictures retain priority. Reduced artwork reads/redraws and removed the experimental Animation switch.
- Preserved complete GB/GG save names and legacy saves/settings during SYSTEM migration; consolidated the settings/history sidecars and retained artwork Off in carousels. Failed plugin loads restore the browser, and recognized SMS/PCE startup code retains safe cartridge timing.
- Improved captions, selection markers and compact settings; framed Help/System/update panels, B at the SD root returns to Start, and firmware unlock uses Down, Up, Left, Right within five seconds.

**Folder artwork:** Extract the attached `folder-art.zip` to the SD card root, merging its `SYSTEM` folder with yours. Title pictures use `SYSTEM/IMGS/CUSTOM`; Box pictures use `SYSTEM/IMGS2/CUSTOM`. Keep your own matching pictures if preferred.

**Hardware and validation:** This remains an unfinished testing port. Earlier Chis builds have owner-confirmed filename/ROM-header artwork and working five-image lookahead. The new NOR workflow and loader changes need physical testing; plugin Exit can still crash, PCE loading and reported settings persistence issues remain unresolved, and complete EZ-FLASH DSSMM parity is still in progress. SD and Lite remain physically untested. Builds, host checks and emulator previews do not qualify flash/save timing or gameplay.

Choose the image for SuperChis/SuperChis Prime, SuperCard SD or SuperCard Lite. Back up saves and current firmware before testing; review [upstream installation and recovery guidance](https://superfw.davidgf.net/docs/install/flash/) before internal-flash writes. Confirming replacement removes existing NOR games; a later write failure cannot restore them.

Credits: David Guillen Fandos/davidgfnet for SuperFW; ChisBread and collaborators for SuperChis; Merkin for this port; DS Style/DSSMM contributors for presentation assets; emulator/plugin authors; DSSCC and its FrankieT19 DS Style scraper lineage for artwork tooling. Component license notices remain in the source.
