## New features

- Chis now displays title artwork by default. Choose List, Title or Box in **UI settings**; Start in the browser still cycles through the views.
- Standard save, savestate and diagnostic files now live under `/SYSTEM`. The private `/.superfw` folder stays in place, and existing files in the old root folders remain usable.

## New fixes

- Older Chis UI settings migrate to the artwork default. Settings saved by this build retain an explicitly selected List mode.
- Existing root-level save sets and savestates are detected and continue using their original paths when no matching `/SYSTEM` copy exists. Nothing is moved or deleted automatically.

## Summary of past features

- Adds DSSCC-compatible name-matched title and box artwork from mirrored and `CUSTOM` folders, plus manual themes and color accents.
- Based on [David Guillen Fandos’s SuperFW](https://github.com/davidgfnet/superfw), retaining its native game loading, patching, saves, cheats, in-game menu and game save states. Chis includes NOR support. Bundled emulators remain upstream versions; this build does not include MiGoomba.
- The interface is still an early port. Consistent DSSMM styling across the browser, settings, system tools, game screens and in-game menus remains in development.

## Summary of past fixes

- Rejects malformed artwork, unsafe or overlong lookup paths and incomplete reads; caches previews to avoid repeated SD access and stale images.
- This is the second DSSMM SuperFW prerelease; earlier notes describe the initial artwork and theme work. Software checks do not establish cartridge compatibility.

**Hardware and validation:** the earlier Chis build chain-loaded on one SuperChis Prime v1.0 and reported SuperFW 0.22; manual themes were available. This build’s artwork display, default setting, save/state paths and broader interface have not been tested on hardware. SuperCard SD and Lite builds also remain untested. Hosted checks passed for all projects and all three SuperFW builds; local Chis, SD and Lite images were built and their headers and firmware seals verified.

Choose the download for your card: SuperChis/SuperChis Prime, SuperCard SD, or SuperCard Lite. Back up your SD card, saves and current firmware. Review [upstream installation and recovery guidance](https://superfw.davidgf.net/docs/install/flash/) before any internal-flash write, confirm your recovery method, and never interrupt save or flash writes. Existing legacy save files are left in place; see the [SYSTEM layout guide](https://github.com/voluptuary/dssmm/blob/superfw-test2/superfw/docs/SYSTEM-LAYOUT.md) and [artwork setup guide](https://github.com/voluptuary/dssmm/blob/superfw-test2/superfw/docs/ARTWORK-SETUP.md).

Credits: David Guillen Fandos/davidgfnet for SuperFW; ChisBread and collaborators for SuperChis hardware; Merkin for this port; DSSCC and its FrankieT19 DS Style scraper lineage for artwork tooling. Firmware and component license notices remain in the source.
