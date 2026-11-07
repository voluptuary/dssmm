# Changelog

## DS Style Merkin Mix 8.3.0

### New features and improvements

- Refined the Start screen, startup presentation, menu sounds and pop-ups. Added eased selection markers, a loading ring and theme-matched launch fade.
- Added a clock-based Auto theme, grouped artwork controls under View, Open SYSTEM access, default wrap-around with saved preferences preserved, and Boot to controls. Added double-tap Select to toggle favourites without a confirmation pop-up.
- Highly optimized menu artwork rendering by skipping redundant transparency scans for opaque artwork, while retaining compatibility with marked transparent thumbnails.
- Improved header-code artwork lookup in large folders with a validated FAT/FAT32 directory-entry fast path. Unsupported filesystems and stale directory hints safely fall back to normal path lookup.
- Added nested-folder artwork support for NOR titles and clearer NOR erase, program, source-file and catalog-save failure messages.
- Added selected-game artwork and clearer status text to the Saving Game Data presentation.

### Fixes

- Corrected save-recovery artwork style selection and Box-art cropping; added support for valid 16-bit title BMPs whose planes field is zero.
- Restored NOR menu button hints, corrected format confirmation and cancellation behavior, added erase progress feedback, and erased the full allocated range when deleting the newest NOR title.
- Fixed Clean list Off display behavior, aligned help headings and LED values, and improved root-level GBA header-art fallback.
- Reduced IWRAM pressure by relocating scale maps and moved infrequently used path buffers to EWRAM, with a checked IRQ-stack margin.
- Improved handling of longer translated NOR errors without overflowing a fixed message buffer.

### Earlier features

- **DS Style v7.5 foundation (FrankieT19):** launcher, themes, artwork workflows, language support, animated carousels, side-preview options and launch pulse. This credit is limited to the v7.5 base documented in [CREDITS.md](CREDITS.md).
- Added horizontal, vertical and List + Art views; favourites and recently played lists; title and box artwork; themes; configurable borders, rounded corners and carousel alignment; and a text/Markdown viewer.
- Added mirrored artwork lookup, built-in and custom thumbnail fallbacks, marked-thumbnail transparency, console icons, wide and square system artwork, and the `CUSTOM Only` rounded-corner option.
- Added optional Remember cheats with localized labels through **exploitz86**, and collapsible cheat groups based on **SkillerCMP's** work and adapted by **exploitz86**. Remember cheats is off by default.
- Added browser wrap settings, Clean list and List folders controls, 12/24-hour clock options, Hide system files, and support for installing kernels from `/SYSTEM/KERNELS`.
- Added Thai and Chinese language support, customizable launcher text and Start screen layouts, full-screen theme support, and the DS Style Merkin Mix branding/build number.
- Added configurable Start screen sources and shortcuts, empty-folder/favourites messages, folder and file custom thumbnails, and built-in guidance for settings and menus.
- Added multi-language settings/help categories, cheat-screen refinements, and improved menu, pop-up, carousel, list, sound and input behavior across the DS Style lineage.

### Earlier fixes

- Preserved marked-art transparency and corrected animated carousel wrap transitions; fixed collapsed-cheat selection, visible-row scrolling and stale-row behavior (including fixes by **exploitz86**).
- Improved SD save and save-state read/write checks, save recovery, long-filename and folder handling, malformed cheat-file bounds, ROM patching across PSRAM banks, and NOR installation/deletion error handling.
- Added checks for oversized fragmented ROM tables, 512-file folders and empty-list navigation; reduced launcher RAM pressure and corrected recent-list limits and software-boot memory state.
- Fixed translated text and settings handling, Chinese and accented-character rendering, custom-background scrolling, help/clock redraws, cheat-screen redraws and carousel border flicker.
- Improved thumbnail loading/redraw, file browsing, menu navigation, startup timing, and settings migration; optimized artwork drawing while retaining marked transparency.
- Corrected save-type labels, language/settings values, game-launch behavior with BIOS disabled, and ROM/save-state fragmentation checks.
- Earlier DS Style releases also added safer file-copy, backup, thumbnail-read, directory-bound and cheat-entry limits, and kept the Omega DE and Original Omega source lines aligned where features overlap.

Thanks to **Copilot**, **exploitz86**, **SkillerCMP**, **Sterophonick**, and the EZ-FLASH team for their contributions to this project and its lineage. FrankieT19 is credited for the DS Style v7.5 foundation only; no later Merkin Mix changes are attributed to him. **aidiadayo** contributed Thai language support. See [CREDITS.md](CREDITS.md) for provenance. These changes were developed with AI assistance.
