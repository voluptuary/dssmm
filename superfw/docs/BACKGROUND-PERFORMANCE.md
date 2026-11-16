# Background rendering and profiling

The DS backgrounds now use a run-fill stream (`DBR1`) rather than VRAM-backreference
LZ tokens. Each 16-bit token stores one pair of 4-bit background palette indices
and a run length of1–256 halfwords. Runs are checked against the remaining frame
before writing. Long runs use SuperFW's existing synchronous GBA DMA3 halfword
fill helper; short tails use aligned halfword writes. No new hardware driver,
framebuffer or GBA cache buffer is added. Idle carousel page/artwork reuse remains.

This retains every source pixel, both Light/Dark palettes and all native font
prefix/header bytes. The resource blob grows from18436 to58652bytes, but the
native UPKR wrapper compresses the standard combined font/background resource
from279494 to280401bytes in the measured comparison (907bytes more). The rounded
extended copy still has458752bytes free in the existing1.5MiB resource partition.
A complete SD/Lite/Chis firmware size check is required; resource-only comparison
is not proof that the firmware fits.

In the same mGBA0.10.1 synthetic vertical scene, a component-timed prototype
measured background700922→310810cycles (about56% less), with artwork502309cycles
unchanged. Total redraw1296968→906856cycles (about30% less). These are ROM-fixture
measurements, not physical flash-card/kernel FPS: the fixture lacks the firmware's
selected-art hot cache and idle-page reuse, and code/art memory mapping differs.
Do not turn these numbers into a hardware qualification claim.

Reproduce current component timings with explicit Light theme and DSSMM font:

```sh
PREVIEW_THEME=6 PREVIEW_FONT=1 PREVIEW_VERTICAL=1 PREVIEW_TIMING=1 \
  bash scripts/test-browser-preview-mgba.sh
```

Timing mode displays background, artwork and total cycles and intentionally
covers part of the test scene. Timer reads are omitted when timing is disabled.
No mGBA binary modification, custom emulator build or private BIOS is required.

## Pixel and bus checks

- `bash scripts/test-dssmm-background.sh`: every stored source pixel and guards,
  all12 backgrounds, using the portable decoder path.
- `python3 scripts/test-background-video-bus.py`: actual ARM embedded/standard/
  extended/missing-tag paths; read-only native resource and halfword-write checks.
- `bash scripts/test-background-mgba.sh`: actual `__GBA__` DMA path in stock SDL
  mGBA under Xvfb; captures12 frames, compares all38400 pixels in each against
  the source, and checks frame-padding canaries. This does not emulate card IO.
- `python3 tests/test-preview-pixels.py`: shared standard-library PNG reader and
  artwork proof. Gray background panels must never count as decoded artwork.

The browser preview now counts chromatic synthetic-art cube colors rather than
all non-UI pixels. The previous heuristic could mistake richer gray DS backgrounds
for artwork. PNG decoding is shared by both screenshot harnesses; no Pillow or
other new package dependency is needed. Emulator package and flags remain stock;
keep any private BIOS outside the repository and release assets.

## Internal-RAM hot functions

The background run decoder and clipped artwork scaler now use the existing
`IWRAM_CODE` placement, with no new image buffer. In the same explicit Light,
DSSMM-font, vertical fixture the counters are BG184241, Art254317 and All532310
cycles, compared with the previous run-fill fixture's 310810/502309/906856.
This is about41% less total fixture time, not a measured physical firmware FPS.
All12 actual mGBA background pixel/guard comparisons pass. Production linked
IWRAM/stack and all3 flash limits must also pass before distributing a build.

## Post-Test5 audit

The unused QR Help background has been removed from the codec/API/assets and
packs; Help is already drawn as an opaque plain panel. DBR2 contains the ten
active Light/Dark backgrounds and a33320-byte blob (25332bytes less), retaining
all native font-prefix bytes. All10 host/ARM/actual-mGBA source pixels/guards
pass. Extended resource headroom is491520bytes; actual firmware gates remain
required. Earlier12-scene/DBR1 measurements above describe released Build624.

The clipped scaler now advances exact integer sample/remainder positions rather
than dividing for every column and row. Fractional up/downscale, transparency and
clipped-edge cases compare against the independent division formula. Same-scene
mGBA art cost254317→147096cycles (~42% less); total532310→425089 (~20% less), with
background184241 unchanged. These are synthetic fixture counters, not physical
firmware FPS. No new cache/image buffer or relaxed hardware budget is involved.
