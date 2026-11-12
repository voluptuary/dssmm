## New features

- Manual Light/Dark themes and independent Blue, Green, Orange and Purple accents. New Chis setups start in Light; existing theme choices are retained.
- SD-browser artwork preview: press **Start** to cycle List → title artwork → box artwork. Use **UI settings → Save** to remember the mode.
- Name-matched [DSSCC](https://github.com/voluptuary/dsscc) artwork from mirrored folders and `CUSTOM` under `/SYSTEM/IMGS` and `/SYSTEM/IMGS2`, including marked transparency. Artwork uses a reduced color palette.

## New fixes

- Reject malformed artwork, incomplete reads and overlong or unsafe lookup paths; preserve opaque black in unmarked images.
- Cache artwork hits and misses to avoid repeated SD access for the same selection, and invalidate previews around directory changes and actions that can reuse memory.

## Summary of past features

- Based on [David Guillen Fandos’s SuperFW](https://github.com/davidgfnet/superfw), retaining its native game loading, patching, saving, cheats and game-dependent in-game menu/save states. NOR library support is available in the Chis variant. Bundled emulators remain upstream versions; this build does not include MiGoomba.
- This is the first DSSMM SuperFW prerelease. It is an early interface port, with further DS-style layout work ahead.

## Summary of past fixes

- Inherits the pinned SuperFW baseline; there is no earlier DSSMM SuperFW release history. Passing builds and software tests do not establish physical cartridge compatibility.

**Hardware untested:** choose only the download for your card:

| Card | Download |
| --- | --- |
| SuperChis / SuperChis Prime (Prime v1.0 is the intended first test card) | `superfw-chis.fw` |
| SuperCard SD | `superfw-sd.fw` |
| SuperCard Lite | `superfw-lite.fw` |

Back up your SD card, saves and current firmware. For a first test, use [upstream’s chain-loading instructions](https://superfw.davidgf.net/docs/install/flash/#no-installation) if supported by your installed firmware; a copy of the matching `.fw` renamed to `.gba` can be launched as homebrew. Confirm your recovery method before installing to internal flash, and never interrupt save or flash writes.

For step-by-step pack creation and installation, see [the artwork setup guide](https://github.com/voluptuary/dssmm/blob/superfw-test1/superfw/docs/ARTWORK-SETUP.md). For a short hardware test checklist, see [the preview guide](https://github.com/voluptuary/dssmm/blob/superfw-test1/superfw/docs/ARTWORK-PREVIEW.md). Folder artwork, GBA code-based packs, additional lookup fallbacks, NOR/recent previews and carousels are not implemented in this prototype. No automatic themes, hardware RTC or rumble controls are offered for Prime.

Credits: David Guillen Fandos/davidgfnet for SuperFW; ChisBread and collaborators for SuperChis hardware; Merkin for this port; DSSCC and its FrankieT19 DS Style scraper lineage for artwork tooling. Firmware and component license notices remain in the source.
