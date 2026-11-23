## New features

- Native MiGoomba GB/GBC emulation on Chis, SD and Lite, with existing save locations and optional external plugin overrides.
- Optional carousel wraparound, a lighter Dark top-bar color, stronger header shading, Interface scroll indicators and refined DS Style menu frames.
- Framed progress screens describe loading and flashing operations. Settings-save notices remain visible instead of flickering between writes.

## New fixes

- Corrected GG/SMS plugin return handling; operation is now reported working on Chis hardware.
- Hardened native emulator transfers with RAM-staged headers, exact reads, aligned final blocks and failure cleanup. PCE can try the native emulator when portable loading fails.
- Reduced packed resource size to recover SD/Lite firmware headroom within the original hardware limits.
- Refined Start selectors, menu text alignment and Dark panel outlines.

## Summary of past features

- DS Style Start screen, username, editable 12/24-hour clock, grouped settings, Light/Dark themes, accents and independent header colors.
- List and artwork carousel browsing with five-image caching, Clean list, filename/CUSTOM art, ROM-header Title art and folder artwork.
- Shared `SYSTEM` settings, recents, saves and plugins; automatic verified settings saves and legacy-folder migration.
- Pogoshell plugins, bundled emulator fallbacks and Chis NOR writing with capacity checks and confirmation.

## Summary of past fixes

- Bounded path handling and patch scanning; guarded ROM/NOR transfers, allocations and error cleanup.
- Corrected settings persistence, cartridge-memory rendering and artwork compatibility; improved menu responsiveness and alignment.

## Known issues and hardware testing

- **Only Chis has been tested on hardware**, on SuperChis Prime. SD/Lite remain physically untested; this build’s new changes need card testing.
- PCE loading remains unconfirmed on hardware. The fallback and transfer corrections pass software tests, but do not establish physical compatibility. Other plugins may require unsupported card behavior.
- Full EZ-FLASH DSSMM visual/performance parity is still in progress. SD operations can stall a frame; compressed-resource startup performance on SD/Lite needs hardware testing.
- Old emulator save-state compatibility is not established. The software clock does not advance while powered off. Confirmed NOR replacement removes existing games.

## SD-card setup

1. Choose the `.fw` for your exact card: Chis, SD or Lite. Back up saves and follow [SuperFW’s installation/recovery guide](https://superfw.davidgf.net/docs/install/flash/).
2. Keep games in your preferred folders. Settings, recents and saves use `SYSTEM/SETTINGS.TXT`, `SYSTEM/RECENT.TXT` and `SYSTEM/SAVER`. An optional username goes in `SYSTEM/NAME.TXT`.
3. Extract **folder-art.zip** to the SD root, merging `SYSTEM`. Title art uses `SYSTEM/IMGS`, Box art `SYSTEM/IMGS2`, and overrides each folder’s `CUSTOM` directory. See [artwork setup](https://github.com/voluptuary/dssmm/blob/dssmm/superfw/docs/ARTWORK-SETUP.md).
4. Optional plugins go in `SYSTEM/PLUG`. Select artwork, font and colors in Settings → Interface; let settings saves finish before powering off.

## Credits

- [davidgfnet](https://github.com/davidgfnet/superfw) for SuperFW.
- [FrankieT19](https://github.com/FrankieT19) for DS Style.
- michyprima, Jaga, Dwedit, FluBBa and the other emulator authors. Component notices remain in the source.
