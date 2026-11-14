## New features

- Title artwork now supports original EZ-FLASH GBA thumbnails matched by the four-letter game code in the ROM header, after filename-matched artwork.
- Settings and option menus use left-aligned text, and the browser bar no longer has decorative DSSMM text over the existing icons.

## New fixes

- Title and Box artwork check the `CUSTOM` filename before the mirrored ROM filename. Title then checks the native EZ-FLASH game-code artwork path.
- Removed the clipped outline around the artwork preview.

## Summary of past features

- All three builds use DSSMM Light and Title artwork by default, with shared theme, accent, and List/Title/Box controls. Chis-only hardware features remain specific to Chis.
- Adds DSSCC-compatible filename-matched title and box artwork, manual themes and color accents, and `/SYSTEM` save, state and diagnostic folders. Existing SuperFW game loading, saves, cheats and in-game menus remain; NOR support is available on Chis.
- Based on [David Guillen Fandos’s SuperFW](https://github.com/davidgfnet/superfw). Bundled emulators remain upstream versions; this port does not include MiGoomba.

## Summary of past fixes

- Older unversioned and version 1 UI settings migrate to DSSMM Light and Title defaults; version 2 settings keep their saved choices.
- Artwork reads are bounded and reject malformed images; DSSCC `DST1` transparency is supported. Existing root-level save and state data remains usable when no matching `/SYSTEM` copy exists.

**Hardware and validation:** Test 4 artwork and menu changes have not been checked on hardware. The earlier Test 1 Chis image booted on one SuperChis Prime v1.0 and showed SuperFW 0.22 with manual themes; that smoke test does not validate artwork. SuperCard SD and Lite also remain physically untested. Hosted checks passed on all five repository branches, and all three board images were built and verified locally.

Choose the download for your card: SuperChis/SuperChis Prime, SuperCard SD, or SuperCard Lite. Back up your SD card, saves and current firmware. Review [upstream installation and recovery guidance](https://superfw.davidgf.net/docs/install/flash/) before any internal-flash write, confirm your recovery method, and never interrupt save or flash writes. Existing legacy save files are left in place; see the [SYSTEM layout guide](https://github.com/voluptuary/dssmm/blob/superfw-test4/superfw/docs/SYSTEM-LAYOUT.md) and [artwork setup guide](https://github.com/voluptuary/dssmm/blob/superfw-test4/superfw/docs/ARTWORK-SETUP.md).

Credits: David Guillen Fandos/davidgfnet for SuperFW; ChisBread and collaborators for SuperChis hardware; Merkin for this port; DSSCC and its FrankieT19 DS Style scraper lineage for artwork tooling. Firmware and component license notices remain in the source.
