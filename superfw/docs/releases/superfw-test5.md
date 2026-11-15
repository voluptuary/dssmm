## New features

- Horizontal and vertical carousels with cached neighboring artwork and scrolling filenames.
- **Animation** switches between sliding transitions and a faster native-size view. Off skips scaling and neighbor artwork reads, shows system icons beside the selected game, and reuses unchanged artwork frames.
- More consistent left-aligned game options, Info and Tools text. Info shows the SuperFW source hash alongside the separate DSSMM build number.

## New fixes

- Moved carousel artwork metadata into GBA RAM so cache flags and filenames no longer use unsupported byte writes to cartridge memory.
- Fixed an invalid selection caused by pressing Left at the first browser entry.
- Reduced artwork read calls and scaling work; added support for bottom-up RGB555 thumbnails, optional zero size fields and the unused RGB555 high bit.
- Title artwork also checks a flat game-code filename after the nested EZ-FLASH path. CUSTOM and mirrored filename matches retain priority.

## Summary of past features

- Shared Light/Dark themes, accents and List/Title/Box artwork controls across Chis, SD and Lite. Supports DSSCC filename artwork and EZ-FLASH GBA game-code Title artwork; Chis retains its own NOR features.
- Uses `/SYSTEM` for new save, state and diagnostic folders while retaining native SuperFW loading, saves, cheats, in-game menus and bundled upstream emulators. MiGoomba is not included.
- Based on [David Guillen Fandos’s SuperFW](https://github.com/davidgfnet/superfw), with DSSMM’s DS Style presentation lineage.

## Summary of past fixes

- Preserves saved preferences when migrating older configurations and retains existing legacy save/state paths where applicable.
- Bounded artwork decoding, CUSTOM-first matching and DSSCC DST1 transparency; removed the clipped artwork border and decorative text over the tab icons.

**Hardware and validation:** This is an unfinished port for hardware testing. On the owner’s SuperChis Prime, Build 553 fixed the browser-edge crash and displayed Box art in List view, but carousel artwork remained absent. An ARM regression reproduces unsupported cartridge byte writes in that build; the cache-memory fix passes that check but still needs physical confirmation. Full DSSMM styling and fluidity remain in progress. SD and Lite remain physically untested. Emulator previews cover isolated rendering only.

Choose your card’s image and back up saves and current firmware. Follow [upstream installation and recovery guidance](https://superfw.davidgf.net/docs/install/flash/) for internal-flash operations. Existing save files stay in place; see the project’s SYSTEM layout and artwork setup guides.

Credits: David Guillen Fandos/davidgfnet for SuperFW; ChisBread and collaborators for SuperChis; Merkin for this port; DSSCC and its FrankieT19 DS Style scraper lineage for artwork tooling. Component license notices remain in the source.
