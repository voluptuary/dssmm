# DSSMM presentation and performance continuation

Primary reference: `ezfomegade/source/ezkernelnew.c` (owner's “EZFODE” DSSMM).
Use presentation/layout as reference; retain SuperFW hardware, load/save, NOR,
emulator and game-state backends. All changes compile for SD, Lite and Chis.
Unsupported hardware features remain excluded.

| Surface | Current implementation | Remaining qualification/work |
| --- | --- | --- |
| SD browser | List/horizontal/vertical, filename artwork, CUSTOM priority, Title code fallback, palette, scrolling labels | Header Title still failed on owner561; legacy fallback needs physical result. Full DE chrome/typography and recent/NOR artwork remain incomplete. |
| Carousel On | DE neighbor positions, clipped scaling, focused-last drawing, no SD reads during motion, settled-page reuse | Physically sluggish on561; later hot-cache/lookup optimizations not measured on card. Fluidity is not established. |
| Carousel Off | Native-size selected/previous/next, screen/body edge crop, no scaling, idle-page reuse | User-requested behavior implemented after561;571 is ready for hardware testing. |
| Artwork storage | Halfword SDRAM pixels; byte metadata in IWRAM; focused image hot copy in EWRAM | 561 filename result confirms metadata fix. Newest focus refinement is newer than571. |
| Main/UI settings and Tools | Bounded left-aligned rows, solid accent selections on Light/Dark; native themes retained | Actual renderers previewed; real input/persistence only host-tested. Long localized strings may truncate. |
| ROM info/patch/loading | Bounded left labels, solid Light/Dark selections, navigation-safe headings, compact English labels/date | Actual renderer previews pass; real launch/NOR actions not exercised by previews. |
| In-game menus | Theme palette inherited; solid selection rows across main/reset/save/RTC update/cheats/states | First four previewed with fake state. Cheats/states action rows need visual fixtures; game-state/save/restore qualification needs real card. Logo, dialogs, typography and slot composition retain native SuperFW elements. |
| Confirmation/RTC/file-manager/system dialogs | Native behavior, inherited palette and earlier alignment fixes | Continue visual consistency, wrapping/bounds and interactive coverage. |

## Hardware evidence

- Chis553: owner confirms first-entry crash fixed; Box art works in List; carousel
  art absent.
- Chis561: owner confirms filename art renders; ROM-header Title does not. On
  remains slow, delayed neighbors; Off had no observed gain in that older design.
- Chis571/hash3d0b1a80: current frozen local `hardware-test` image. Includes native
  cropped neighbors, settled reuse, faster color conversion, selected EWRAM copy,
  legacy code-art record fallback and aligned clipped scaling. No owner result yet.
- SD/Lite have no physical qualification. Latest development UI and visual-focus
  cache refinement are newer than571. See numbered validation JSON and journals.

## Performance constraints and next checks

1. Measure571 on the same card and artwork pack before claiming a speed gain.
2. Storage reads remain synchronous. Large FAT directories/path probes and
   initial three-image loading may stall input even when drawing is fast.
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

Keep build571 unchanged until superseded with a clean numbered all-board set.
Preserve BIOS and session scratch outside the repository until the whole session
ends. Keep journals current and remove superseded generated artifacts after
retaining evidence. No full parity, gameplay or physical timing claim yet.
