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

**Hardware testing:** the Chis build has been chain-loaded successfully on one SuperChis Prime v1.0. It reported SuperFW 0.22, and manual themes are available. The SD and Lite builds have not been tested on hardware. This is an initial smoke test, not full hardware qualification.

| Card | Download |
| --- | --- |
| SuperChis / SuperChis Prime (Prime v1.0 is the intended first test card) | `superfw-chis.fw` |
| SuperCard SD | `superfw-sd.fw` |
| SuperCard Lite | `superfw-lite.fw` |

The overall menu still uses the upstream SuperFW layout; the full DSSMM-style interface is still in development. Theme availability and successful chain-loading do not validate gameplay, saves, artwork rendering, NOR operations or installation. Back up your SD card, saves and current firmware. Review [upstream installation and recovery guidance](https://superfw.davidgf.net/docs/install/flash/) before any internal-flash write, confirm your recovery method, and never interrupt save or flash writes.

For step-by-step pack creation and installation, see [the artwork setup guide](https://github.com/voluptuary/dssmm/blob/superfw-test1/superfw/docs/ARTWORK-SETUP.md). For a short hardware test checklist, see [the preview guide](https://github.com/voluptuary/dssmm/blob/superfw-test1/superfw/docs/ARTWORK-PREVIEW.md). Folder artwork, GBA code-based packs, additional lookup fallbacks, NOR/recent previews and carousels are not implemented in this prototype. No automatic themes, hardware RTC or rumble controls are offered for Prime.

Credits: David Guillen Fandos/davidgfnet for SuperFW; ChisBread and collaborators for SuperChis hardware; Merkin for this port; DSSCC and its FrankieT19 DS Style scraper lineage for artwork tooling. Firmware and component license notices remain in the source.
