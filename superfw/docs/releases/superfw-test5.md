## New features

- DS Style Start screen with last-played artwork, `SYSTEM/NAME.TXT` username, a selectable clock, SD/NOR shortcuts and grouped settings. Adds the optional DSSMM font and Clean list names without NoIntro tags.
- Horizontal and vertical carousels with five-image artwork lookahead, retained settled frames and faster background/artwork rendering. CUSTOM folder pictures include global `#` and A–Z matches.
- One `SYSTEM/SETTINGS.TXT` combines shared EZ-FLASH settings with SuperFW options; settings save when leaving a menu. Recent history uses `SYSTEM/RECENT.TXT`, new saves use `SYSTEM/SAVER`, and emulator plugins use `SYSTEM/PLUG`.
- Extension-named BIN/GBA/MB/MBZ emulator plugins can precede native fallbacks. Game Gear uses the SMS plugin with its own cartridge icon. Recognized SMSAdvance/PCEAdvance startup code keeps SuperCard-safe cartridge timing.

## New fixes

- Fixed browser-edge crashes, cartridge-memory artwork cache writes and legacy EZ-FLASH ROM-header Title image compatibility. CUSTOM and filename artwork retain priority.
- Reduced repeated artwork reads, scaling and redraws; neighboring pictures refresh after loading. Removed the experimental Animation switch.
- Preserved complete GB/GG save basenames, including version dots, with safe fallback to existing shortened saves. Failed emulator loads rebuild browser state before displaying an error.
- Preserved artwork Off in carousel settings and safely migrated the old UI-settings and recent-history sidecars. Broader reported settings loss remains under investigation.
- Tidied carousel captions, Start selection corners and compact System rows. Help, System and firmware-update screens use plain framed panels; B at the SD root returns to Start. Update unlock is Down, Up, Left, Right within five seconds.

## Summary of past features

- Shared Light/Dark themes, accents and List/Title/Box artwork controls across Chis, SD and Lite; DSSCC filename artwork and EZ-FLASH GBA game-code Title artwork. Chis retains its own NOR support.
- Based on [David Guillen Fandos’s SuperFW](https://github.com/davidgfnet/superfw), retaining native loading, saves, cheats, in-game menus and bundled emulators. MiGoomba is not included.

## Summary of past fixes

- Bounded artwork decoding, CUSTOM-first lookup and DSSCC transparency; removed the clipped artwork border and decorative text over menu icons.
- Preserved older preferences and existing legacy save/state paths during SYSTEM migration.

**Folder artwork:** Extract the attached `folder-art.zip` to the SD card root, merging its `SYSTEM` folder with yours. Pictures go into `SYSTEM/IMGS/CUSTOM` for Title and `SYSTEM/IMGS2/CUSTOM` for Box; keep your own matching pictures if preferred. The 120×80 letter pictures retain the original folder shape on a wider transparent canvas.

**Hardware and validation:** This remains an unfinished port for testing. Earlier Chis hardware builds now display filename and ROM-header artwork, and the browser-edge crash is fixed. The owner reports improving menu performance, but full EZ-FLASH DSSMM parity is still in progress. The new timing correction needs physical testing; plugin Exit can still crash, and reported settings persistence issues are not fully resolved. SD and Lite remain physically untested. Local builds, host checks and emulator previews do not qualify flash/save timing or gameplay.

Choose the image for SuperChis/SuperChis Prime, SuperCard SD or SuperCard Lite. Back up saves and current firmware before testing; review [upstream installation and recovery guidance](https://superfw.davidgf.net/docs/install/flash/) before internal-flash writes.

Credits: David Guillen Fandos/davidgfnet for SuperFW; ChisBread and collaborators for SuperChis; Merkin for this port; DS Style/DSSMM contributors for presentation assets; emulator/plugin authors; DSSCC and its FrankieT19 DS Style scraper lineage for artwork tooling. Component license notices remain in the source.
