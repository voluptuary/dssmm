## New features

- DSSMM Light and Title artwork are now the defaults on SuperCard SD and SuperCard Lite as well as Chis, keeping the shared interface consistent across all three builds.
- All variants expose the same theme, accent and List/Title/Box settings, use the same artwork lookup and follow the same `/SYSTEM` save/state layout where supported.

## New fixes

- Older unversioned and version 1 UI settings migrate from the former native defaults to DSSMM Light and Title artwork on every board.
- Test 2 version 2 settings retain their saved choices, including an explicitly selected native theme or List view.

## Summary of past features

- Adds DSSCC-compatible name-matched title and box artwork, manual themes and color accents, plus grouped `/SYSTEM` save, state and diagnostic paths. The private `/.superfw` data remains separate.
- Based on [David Guillen Fandos’s SuperFW](https://github.com/davidgfnet/superfw), retaining its native game loading, patching, saves, cheats, in-game menu and game save states. NOR support is available on Chis. Bundled emulators remain upstream versions; this build does not include MiGoomba.
- The DSSMM browser treatment has begun, but consistent styling of all menus and game screens is still in development.

## Summary of past fixes

- Rejects malformed artwork and unsafe or incomplete file reads, and caches previews to avoid repeated SD access or stale images.
- Existing root-level saves and states remain usable when no matching `/SYSTEM` copy exists. Prior notes describe the first artwork and settings work.

**Hardware and validation:** all three variants passed hosted checks and were built for this prerelease, but the new SD/Lite defaults and artwork display have not been checked on those cards. No Test 3 changes have been physically validated. An earlier Chis image booted on one SuperChis Prime v1.0 and showed manual themes; that Test 1 smoke test does not establish Test 3 artwork, save/state or interface behavior. Chis-only NOR and other hardware-specific capabilities remain distinct.

Choose the download for your card: SuperChis/SuperChis Prime, SuperCard SD, or SuperCard Lite. Back up your SD card, saves and current firmware. Review [upstream installation and recovery guidance](https://superfw.davidgf.net/docs/install/flash/) before any internal-flash write, confirm your recovery method, and never interrupt save or flash writes. Existing legacy save files are left in place; see the [SYSTEM layout guide](https://github.com/voluptuary/dssmm/blob/superfw-test3/superfw/docs/SYSTEM-LAYOUT.md) and [artwork setup guide](https://github.com/voluptuary/dssmm/blob/superfw-test3/superfw/docs/ARTWORK-SETUP.md).

Credits: David Guillen Fandos/davidgfnet for SuperFW; ChisBread and collaborators for SuperChis hardware; Merkin for this port; DSSCC and its FrankieT19 DS Style scraper lineage for artwork tooling. Firmware and component license notices remain in the source.
