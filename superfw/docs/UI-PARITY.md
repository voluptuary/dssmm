# DSSMM presentation and performance continuation

Primary reference: `ezfomegade/source/ezkernelnew.c` (owner's “EZFODE” DSSMM).
Use presentation/layout as reference; retain SuperFW hardware, load/save, NOR,
emulator and game-state backends. All changes compile for SD, Lite and Chis.
Unsupported hardware features remain excluded.

| Surface | Current implementation | Remaining qualification/work |
| --- | --- | --- |
| SD browser | List/horizontal/vertical, filename artwork, CUSTOM priority, Title code fallback, palette, scrolling labels | Header Title still failed on owner561; legacy fallback needs physical result. Full DE chrome/typography and recent/NOR artwork remain incomplete. |
| Carousel | DE neighbor positions, clipped scaling, focused-last drawing, no SD reads during motion, settled-page reuse | Physically sluggish on561; later hot-cache/lookup optimizations not measured on card. Fluidity is not established. |
| Artwork storage | Halfword SDRAM pixels; byte metadata in IWRAM; focused image hot copy in EWRAM | 561 filename result confirms metadata fix. Current cache is selected±2; outer prefetch avoids redraw/copy. |
| Main/UI settings and Tools | Bounded left-aligned rows, solid accent selections on Light/Dark; native themes retained | Actual renderers previewed; production UI-settings input exercised in mGBA for toggles, scrolling and top bound. Storage/filter boundaries are mocked; persistence remains host-tested. Long localized strings may truncate. |
| ROM info/patch/loading | Bounded left labels, solid Light/Dark selections, navigation-safe headings, compact English labels/date | Actual renderer previews pass; real launch/NOR actions not exercised by previews. |
| In-game menus | Theme palette inherited; solid selection rows across main/reset/save/RTC update/cheats/states | All six renderers previewed with fake state, including memory/disk/make-persistent views. Game-state/save/restore qualification needs real card. Logo, dialogs, typography and slot composition retain native SuperFW elements. |
| Confirmation/RTC/file-manager/system dialogs | Native behavior, inherited palette and earlier alignment fixes | Continue visual consistency, wrapping/bounds and interactive coverage. |

## Hardware evidence

- Chis553: owner confirms first-entry crash fixed; Box art works in List; carousel
  art absent.
- Chis561: owner confirms filename art renders; ROM-header Title does not. On
  remains slow, delayed neighbors; Off had no observed gain in that older design.
- Chis571/hash3d0b1a80: superseded local `hardware-test` image. Includes native
  cropped neighbors, settled reuse, faster color conversion, selected EWRAM copy,
  legacy code-art record fallback and aligned clipped scaling. No owner result was reported.
- SD/Lite have no physical qualification. Chis582/hash842bad38 is the current frozen hardware-test image; it includes all
  UI/focus-cache refinements plus aligned BMP-tail staging. Owner582 reports only AGS/TCHK header Title, improving but slow navigation, and brief neighbor fallback icons. New reader/lookahead changes need physical confirmation. See numbered validation JSON and journals.

## Performance constraints and next checks

1. Compare the next numbered build with the owner582 result before claiming a speed gain.
2. Storage reads remain synchronous. Large FAT directories/path probes and
   initial five-image loading may stall input even when drawing is fast.
3. Animated scaling is software; DMA accelerates opaque/native transfers.
   Focused pixels now use GBA EWRAM, and idle frames do not repeatedly scale.
   A fully affine/OAM renderer would require a separate video-layout design;
   the current Mode4 framebuffer also contains UI, so transforming its BG layer
   would transform the UI too. Preserve the working renderer during exploration.
4. Current Chis linker allocation is about98% of the251KiB EWRAM region because
   the hot buffer reserves9600 bytes. Linker/ELF gates enforce region limits;
   this is not physical stack-peak evidence. Avoid adding more internal image
   buffers without reviewing memory.
5. Official IMGS download remains HTTP403. Legacy fallback matches the DE fixed
   record reader with synthetic malformed-metadata fixtures; no actual original
   pack inspection has occurred. Keep filename parsing strict and CUSTOM first.

Keep build582 unchanged until superseded with a clean numbered all-board set.
Preserve BIOS and session scratch outside the repository until the whole session
ends. Keep journals current and remove superseded generated artifacts after
retaining evidence. No full parity, gameplay or physical timing claim yet.

Animation/instant-mode setting was removed at owner request; settings6 ignores the
retired key while preserving other choices. Normal carousel transitions remain.
See HEADER-ART-COMPATIBILITY.md for exact DE commit944ac13 and native code-art
semantics; the original pack has not been downloaded successfully.

## Current launcher slice (after588)

Owner requests continuous autonomous progression to DE look/feel parity, including
Start, backgrounds, menus/settings. Development now has exact Light/Dark Start
assets, four-panel navigation/last-played thumbnail, a five-category settings hub,
functional page headers instead of DSSMM icon tabs, and DE striped page texture.
Read LAUNCHER-PORT.md. Category children still retain native flat settings,
system tools, Info/logo and dialogs: full parity is explicitly unfinished.
Storage owner update consolidates all resources into SYSTEM with no directory
choices and safe resumable no-overwrite migration; see SYSTEM-LAYOUT.md.

588 is the last distributed candidate until a new clean all3 set passes gates.
592 storage-only packages were frozen but not distributed; hosted43721652107
stalled and was canceled. Native host ASAN startup is intermittent; host test
Makefile now uses non-PIE executables with sanitizers retained, matching the
successful local reruns. Verify that standard local/hosted suite passes this fix.

## Owner594 feedback and next candidate

Owner reports good progress on594. Name/time bar, bold corners, wrapped
Last Played/SD-or-NOR/Settings/Time navigation and NOR B→Start are implemented
in development. Games/Hardware now have independent filtered children and
shared physical-option/input mapping. Time is an explicitly requested software
menu clock, not hardware wall time; see LAUNCHER-PORT.md. Read-only backgrounds
move into the existing font resource partition to recover GBA RAM, retaining
the five-image cache. These changes need clean all3/CI and hardware retest.
System/Help, dialog composition and broader settings isolation remain unfinished.


Build638 follow-up (candidate, physical retest pending): reusable lined/beveled
panels derive from the original Settings right-panel pixels, preserve the header
and clear the carousel for ROM/NOR launch, questions and firmware update. Help
restores the graphical logo at separate palette236..238. System rows are centered
vertically/left-aligned. NOR currently explicitly uses List chrome/footer;
carousel support remains a parity target. Horizontal caption text is inset3px
at each side with an even-aligned156px clear region. Start recent top corners+1y,
clock left corners+2x; GG cartridge13px tall/full16px wide, blank label/flat bottom.
Settings now derive only from canonical SETTINGS.TXT and save on changes/exits
with readback/rollback; physical powercycle persistence is not yet confirmed.

## 2026-11-21 native visual-reference checkpoint

DE, Original OMEGA and BLUE now have native nine-scene mGBA harnesses in their
own projects, integrated through PR13/14/15 after all four CI checks passed.
The shared root comparison tool creates side-by-side/opacity-overlay HTML from
two240x160 captures. Stock mGBA0.10.1 is unchanged; no BIOS is required. SuperFW
native preview --reference holds Test User/12:34:56 and removes test markers.
A local Start comparison is build/de-superfw-start-comparison.html.

New SuperFW behavior includes optional wrapped carousel browsing with the same
five-image cache, consistent centered Info/System headers, a visible blocking
settings-snapshot notice and lined NOR progress/capacity presentation. Native
NOR allocation/write/verification remains unchanged. Full DE parity is unfinished:
next visual pass should match Start fallback artwork and recent-caption vertical
centering, then compare category/settings/dialog spacing against native captures.
NOR/Recents carousel support and hardware performance remain separate goals.
Owner585/657 confirmed artwork/cache and settings persistence on Chis; the new
plugin return path, PCE loading, SD/Lite and new menus remain physically unqualified.
