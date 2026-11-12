# DSSCC artwork format audit — initial findings

Inspected upstream voluptuary/dsscc at 38424ceb250c0a7de3de322b0e6df0bc3906d909 on 2026-11-12. Source: https://github.com/voluptuary/dsscc/tree/38424ceb250c0a7de3de322b0e6df0bc3906d909 . This is an input-format review, not implemented compatibility.

`ezflash_gba_thumbs.py:write_ezflash_bmp` writes a 54-byte BMP header, 40-byte DIB, one plane, 16 bits/pixel, compression 0 and negative height (top-down). Pixels are little-endian GBA RGB555 words: red in bits 0–4, green 5–9, blue 10–14. Generic desktop BMP channel conventions must not be assumed. Rows are padded to a multiple of four bytes.

Bytes 6–9 contain DST1 only for marked transparency. For these files, word zero is the transparency key. Unmarked files must keep zero opaque. The generator lifts visible near-black pixels to (1,1,1) in five-bit channels to avoid collision with the key. Alpha below 128 becomes transparent.

The application’s output_path_for_custom_name creates CUSTOM/<validated-name>.bmp. Full filename validation, manifest generation/encoding, mirrored paths, kernel precedence and Unicode/case behavior still need auditing before writing the lookup layer. Do not infer the complete lookup contract from this one output helper.

Implementation direction: bounded header/dimension/offset/file-length checks, exact reads and external SD storage. Native menu frames are palette-indexed; plan and measure a bounded RGB555-to-frame presentation strategy before allocating full artwork caches or promising faithful color/transparency. Preserve third-party attribution if code is adapted. Initial target sizes remain 120×80 title and 80×80 box.
