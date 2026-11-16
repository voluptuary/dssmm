# Background rendering and profiling

Ten active Light/Dark backgrounds use the DBR2 run-fill resource pack. Each
16-bit token stores two4-bit palette indices and a run length of1–256 halfwords.
The decoder bounds each run against the remaining frame; DMA3 fills long runs,
and aligned halfword stores handle short tails. Background/scaler hot functions
run in IWRAM. Existing settled-frame and selected-image reuse remain unchanged.
No new image buffer or hardware driver is added.

The native font prefixes and header sizes remain byte-identical. Retiring unused
QR Help streams reduces the appended blob from58652 to33320bytes and leaves
491520bytes in the rounded extended-font resource partition. Help now uses an
opaque framed panel. Build624 used DBR1/twelve backgrounds; historical validation
records still describe that released format. Firmware flash, EWRAM and the16KiB
minimum IWRAM stack are checked separately on every board.

## Measured fixture costs

Same stock mGBA0.10.1 vertical scene, explicit Light theme and DSSMM font:

| Rendering stage | Background cycles | Artwork cycles | Total cycles |
| --- | ---: | ---: | ---: |
| Original LZ fixture |700922|502309|1296968|
| Run-fill with DMA |310810|502309|906856|
| IWRAM hot functions (Build624) |184241|254317|532310|
| Exact remainder scaler |184241|147096|425089|

The remainder scaler replaces per-column/per-row division while preserving exact
integer sampling, fractional scaling and clipped edges. Its art cost is about42%
lower and total fixture cost about20% lower than624. These are synthetic ROM
fixture cycles, not physical firmware FPS: code/art mapping differs, and the
fixture lacks the firmware's selected-art hot cache and idle-frame reuse.

Reproduce counters:

```sh
PREVIEW_THEME=6 PREVIEW_FONT=1 PREVIEW_VERTICAL=1 PREVIEW_TIMING=1 \
  bash scripts/test-browser-preview-mgba.sh
python3 scripts/read-preview-timing.py build/mgba/browser-preview-0.png
```

Timing mode clears its debug rectangle before printing. The exact-glyph reader
avoids manual transcription ambiguity; dark counters require `--dark`. Timer
reads are absent when timing mode is disabled. No private BIOS or emulator patch
is needed.

## Correctness checks

- `test-artwork.sh`:132 fractional/clipped/transparency scenes compare every
  visible pixel and guard against the independent division formula.
- `test-dssmm-background.sh`: all ten decoded source frames and padding guards.
- `test-background-video-bus.py`: actual ARM embedded/standard/extended/missing-tag
  paths, read-only font assets and halfword video writes.
- `test-background-mgba.sh`: actual GBA DMA path in stock SDL mGBA under Xvfb;
  compares all38400 pixels and frame guards for each of ten scenes.
- `tests/test-preview-pixels.py`: standard-library PNG decoding and a chromatic
  artwork oracle, so gray background panels cannot be mistaken for artwork.

The shared PNG reader and glyph matching use no Pillow or OCR package. Keep any
private BIOS outside the repository/releases and document changes to the stock
emulator/harness setup in root local-development records.
