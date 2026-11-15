# Letter-folder artwork pack

The generated pack includes # and A–Z closed-folder sprites in both sizes:
120×80 Title and80×80 Box. Copy its `SYSTEM` folder to the SD-card root and merge
folders. Select Title or Box in Interface settings. Existing CUSTOM images with
those names can be kept as personal overrides. The standard alpha PNG copies
are for editing/preview; the kernel uses the BMPs.

Title files go in `SYSTEM/IMGS/CUSTOM`, Box in `SYSTEM/IMGS2/CUSTOM`.
The existing directory CUSTOM lookup matches only the complete basename:
`/E`, `/Game Boy Advance/E`, and `/Other/E` all use `E.bmp`. It is independent
of parent directories. A longer name such as `Europe` does not match `E.bmp`.
There is no new lookup setting and no firmware/RAM cost for this SD-card pack.
SuperFW588 and newer support CUSTOM directory artwork.

The closed-folder geometry is generated deterministically with centered credited
DE bitmap letters. Transparent pixels use DSSCC's DST1 RGB555 convention; PNG
copies have standard alpha. Both component licenses accompany the ZIP.
Run `python3 tools/generate-letter-folders.py` to regenerate the pack and PNGs.
The verifier decodes all54 BMPs through the production decoder with sanitizers,
checking dimensions, transparency, color variation and guarded output. Physical
folder artwork is still awaiting owner observation; owner confirms588 font good.
