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

## Lookup findings for the next slice

Current DE source Launcher_LoadCustomThumbManifest builds a runtime hash index by scanning CUSTOM; it does not require a separate on-disk manifest file. Its bounded index has 256 entries per style and ASCII-lowercased FNV-1a hashes, followed by exact-path file opens. Prime can avoid inheriting that RAM/index limit by using bounded direct candidate opens; decide after measuring SD latency. Preserve pack filenames rather than invent a new manifest requirement.

Pinned DSSCC thumbnail_output_paths.py mirrors relative ROM directories below IMGS/IMGS2 and strips the ROM extension by default. The standalone CUSTOM helper creates <validated-name>.bmp and rejects names longer than96characters and invalid path characters. Kernel fallback/extension precedence and encoding differences still require review; these findings alone do not define complete compatibility.

## Browser preview integration

The subsequent checkpoint adds artwork_file.c, a selected-file cache in cartridge SDRAM and Start-selectable title/box display in the SD browser. Read ARTWORK-PREVIEW.md for supported paths and omissions. Earlier “not wired” statements describe the decoder-only checkpoint. The native lookup precedence reviewed for this slice is mirrored path then CUSTOM stem. Additional DSSMM fallbacks are still pending. Halfword writes are required for both cartridge cache and framebuffer; artwork_decode now explicitly requires aligned output. Production compositor and exact-read adapter are covered by host tests, with physical visuals still outstanding.
