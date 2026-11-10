# Changelog

## DS Style Merkin Mix 8.3.1

### New features and improvements

- Gave NOR game writing a DS Style progress window with a percentage and progress bar, a dedicated background, and the usual username and clock on the top bar.

### Fixes

- Removed the brief black progress strip during external-plugin launches, giving games such as NES and Game Gear titles the same clean themed transition as GB/GBC launches.

- Made PSRAM game and emulator loading safer: oversized files and combinations that exceed available memory are rejected, and incomplete or failed SD reads stop the launch instead of starting a partial image.
- Corrected loading across PSRAM memory boundaries, including games placed after emulator plugins, and cleared unused space at the end of partial loading blocks for consistent results.
- Fixed thumbnail cache handling when switching to list views, preventing artwork overwritten by list drawing from being reused as a valid cached image.

The PSRAM audit build was tested by the owner on both supported Omega Definitive Edition Rev. A and Rev. B cartridges and reported to work normally. The owner also tested the updated plugin transitions and NOR writing presentation and reports that they work well.

### Earlier features

- Integrated FrankieT19's separate v8.0 Start-screen and interface work: refined selection markers, menu sounds, and pop-ups. Added a loading ring, and a theme-matched launch fade.
- Integrated FrankieT19's v8.0 Auto theme, grouped artwork controls under View, Open SYSTEM access, Boot to controls, and double-tap Select for favorites.
- Highly optimized menu artwork rendering by skipping redundant transparency scans for opaque artwork, while retaining compatibility with marked transparent thumbnails.
- Made header-code artwork appear faster in large game folders, with a safe fallback when the fast lookup is unavailable.
- Added artwork support for NOR games in nested folders and clearer messages when NOR installation, erasing or catalog updates fail.
- Added selected-game artwork and clearer status text to the Saving Game Data presentation, based on FrankieT19's v8.0 work.
- **DS Style v7.5 foundation (FrankieT19):** launcher, themes, artwork workflows, language support, animated carousels, side-preview options and launch pulse.
- Added horizontal, vertical and List + Art views; favorites and recently played lists; title and box artwork; themes; configurable borders, rounded corners and carousel alignment; and a text/Markdown viewer.
- Added mirrored artwork lookup, built-in and custom thumbnail fallbacks, marked-thumbnail transparency, console icons, wide and square system artwork, and the `CUSTOM Only` rounded-corner option.
- Added optional Remember cheats with localized labels through **exploitz86**, and collapsible cheat groups based on **SkillerCMP's** work and adapted by **exploitz86**. Remember cheats is off by default.
- Added browser wrap settings, Clean list and List folders controls, 12/24-hour clock options, Hide system files, and support for installing kernels from `/SYSTEM/KERNELS`.
- Added Thai and Chinese language support, customizable launcher text and Start screen layouts, full-screen theme support, and the DS Style Merkin Mix build number.
- Added configurable Start screen sources and shortcuts, empty-folder/favorites messages, folder and file custom thumbnails, and built-in guidance for settings and menus.
- Added multi-language settings/help categories, cheat-screen refinements, and improved menu, pop-up, carousel, list, sound and input behavior across the DS Style lineage.

### Earlier fixes

- Made settings updates and game-save exports safer by staging the new file first and retaining the previous complete file if an SD write fails.
- Corrected artwork selection and alignment in the startup save-recovery screen, and improved compatibility with valid title artwork from more image tools.
- Restored NOR menu button hints; improved format confirmation, cancellation and progress feedback; and ensured deleting the newest NOR game clears its saved data.
- Restored "Clean list" on\off options, aligned help headings and LED values, and restored header-code artwork for games in the SD-card root.
- Improved display of longer translated NOR error messages.
- Preserved marked-art transparency and corrected animated carousel wrap transitions; fixed collapsed-cheat selection, visible-row scrolling and stale-row behavior (including fixes by **exploitz86**).
- Improved SD save and save-state handling, save recovery, long filenames and folders, malformed cheat-file handling, ROM patching and NOR installation/deletion feedback.
- Improved compatibility with large or fragmented SD folders, corrected recent-list limits and empty-list navigation, reduced menu memory use, and fixed software-boot behavior.
- Fixed translated text and settings handling, Chinese and accented-character rendering, custom-background scrolling, help/clock redraws, cheat-screen redraws and carousel border flicker.
- Improved thumbnail loading/redraw, file browsing, menu navigation, startup timing, and settings migration; optimized artwork drawing while retaining marked transparency.
- Corrected save-type labels, translated settings, game launching with BIOS disabled, and handling of fragmented game and save-state files.
- Earlier DS Style releases also improved file copy, backups, thumbnail loading, folder browsing and cheat-file stability across supported kernel variants.

Thanks to **FrankieT19** for the DS Style v7.5 foundation and separately authored v8.0 interface features listed above; to **exploitz86**, **SkillerCMP**, **Sterophonick**, and the EZ-FLASH team for their contributions to this project and its lineage. **aidiadayo** contributed Thai language support. See [CREDITS.md](CREDITS.md) for provenance.

This project was developed with AI assistance.
