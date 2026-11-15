## New features

- Horizontal and vertical carousels with cached neighboring artwork and scrolling filenames.
- **Animation** switches between sliding transitions and a faster native-size view. Off shows native-size artwork with neighboring images cropped at the edges. Both modes reuse settled artwork frames.
- Accent selection rows in Light/Dark UI settings and Tools, with more consistent left-aligned game options, Info and Tools text. In-game menus share solid selection rows. Info shows the SuperFW source hash alongside the separate DSSMM build number.

## New fixes

- Moved carousel artwork metadata into GBA RAM so cache flags and filenames no longer use unsupported byte writes to cartridge memory.
- Fixed an invalid selection caused by pressing Left at the first browser entry.
- Reduced artwork read calls, repeated scaling and cartridge reads; added support for bottom-up RGB555 thumbnails, optional zero size fields and the unused RGB555 high bit.
- Title artwork checks a flat game-code filename after the nested EZ-FLASH path and supports legacy fixed-size EZ records with inconsistent BMP headers. CUSTOM and mirrored filename matches retain priority.
- Neighbor artwork refreshes when loading finishes; animated previews retain DE positions while clipping safely at screen edges.

## Summary of past features

- Shared Light/Dark themes, accents and List/Title/Box artwork controls across Chis, SD and Lite. Supports DSSCC filename artwork and EZ-FLASH GBA game-code Title artwork; Chis retains its own NOR features.
- Uses `/SYSTEM` for new save, state and diagnostic folders while retaining native SuperFW loading, saves, cheats, in-game menus and bundled upstream emulators. MiGoomba is not included.
- Based on [David Guillen Fandos’s SuperFW](https://github.com/davidgfnet/superfw), with DSSMM’s DS Style presentation lineage.

## Summary of past fixes

- Preserves saved preferences when migrating older configurations and retains existing legacy save/state paths where applicable.
- Bounded artwork decoding, CUSTOM-first matching and DSSCC DST1 transparency; removed the clipped artwork border and decorative text over the tab icons.

**Hardware and validation:** This is an unfinished port for hardware testing. The owner confirms the browser-edge crash is fixed and filename artwork renders on SuperChis Prime Build 561. ROM-header Title artwork still failed, and animation remained slow; later performance and legacy-format changes need physical retesting. Full DSSMM styling and fluidity remain in progress. SD and Lite remain physically untested. Emulator previews cover isolated rendering only.

Choose your card’s image and back up saves and current firmware. Follow [upstream installation and recovery guidance](https://superfw.davidgf.net/docs/install/flash/) for internal-flash operations. Existing save files stay in place; see the project’s SYSTEM layout and artwork setup guides.

Credits: David Guillen Fandos/davidgfnet for SuperFW; ChisBread and collaborators for SuperChis; Merkin for this port; DSSCC and its FrankieT19 DS Style scraper lineage for artwork tooling. Component license notices remain in the source.
