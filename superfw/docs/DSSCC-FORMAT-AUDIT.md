# DSSCC artwork format audit — initial findings

Inspected upstream voluptuary/dsscc at 38424ceb250c0a7de3de322b0e6df0bc3906d909 on 2026-11-12. Source: https://github.com/voluptuary/dsscc/tree/38424ceb250c0a7de3de322b0e6df0bc3906d909 . The bounded BMP decoder is now implemented and tested against this writer; full browser/pack compatibility remains pending.

`ezflash_gba_thumbs.py:write_ezflash_bmp` writes a 54-byte BMP header, 40-byte DIB, one plane, 16 bits/pixel, compression 0 and negative height (top-down). Pixels are little-endian GBA RGB555 words: red in bits 0–4, green 5–9, blue 10–14. Generic desktop BMP channel conventions must not be assumed. Rows are padded to a multiple of four bytes.

Bytes 6–9 contain DST1 only for marked transparency. For these files, word zero is the transparency key. Unmarked files must keep zero opaque. The generator lifts visible near-black pixels to (1,1,1) in five-bit channels to avoid collision with the key. Alpha below 128 becomes transparent.

The application’s output_path_for_custom_name creates CUSTOM/<validated-name>.bmp. Full filename validation, manifest generation/encoding, mirrored paths, kernel precedence and Unicode/case behavior still need auditing before writing the lookup layer. Do not infer the complete lookup contract from this one output helper.

Implementation direction: bounded header/dimension/offset/file-length checks, exact reads and external SD storage. Native menu frames are palette-indexed; plan and measure a bounded RGB555-to-frame presentation strategy before allocating full artwork caches or promising faithful color/transparency. Preserve third-party attribution if code is adapted. Initial target sizes remain 120×80 title and 80×80 box.

## Decoder implementation checkpoint

src/artwork.c reads via an exact random-access callback, accepts only 120×80 or 80×80 top-down uncompressed 16-bit DSSCC BMPs, validates sizes/header/offsets and rejects non-RGB555 high bits. It uses a 240-byte row scratch buffer and caller-owned indexed output (at most 9600 bytes), with no heap or global cache. Output is invalid on any read/parse failure and must not be displayed until success. A future FatFs adapter must translate short/error reads to false.

A fixed 6×6×6 RGB cube occupies background palette entries 20–235, leaving native menu/logo and in-game palette slots intact. Conversion is lossy nearest-level quantization. Index zero is the transparent sentinel; opaque black is palette index 20. No palette upload, VRAM write, file lookup or browser wiring exists yet, and no extra static image buffer has been allocated. GBA VRAM writes require the existing 16-bit bus discipline; do not copy this byte-index output to VRAM using byte stores.

scripts/test-artwork.sh runs actual decoder tests under address/undefined sanitizers for both dimensions, markers, channels, output canaries, malformed headers, buffer limits and read failures. tests/test-dsscc-writer.py optionally executes the pinned upstream writer function with a small image adapter and checks actual generated BMPs using the production decoder, without Pillow or network. Both pass locally at this checkpoint. Native menu theme tests also pass. The optional check takes an external DSSCC source checkout; normal CI has no dependency on it.

Next: finish name/manifest lookup audit, add exact FatFs reads, decide bounded cache lifetime in cartridge RAM and wire a selectable List + Art prototype. Validate palette upload and opaque/transparent composition on an emulator or hardware before claiming visual compatibility. The fixed cube may need refinement after screenshots; retain the external DSSCC files unchanged.
