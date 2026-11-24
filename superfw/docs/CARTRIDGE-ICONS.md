# Native cartridge icons

Original DSSMM cartridge sprites match the native SuperFW bevels/palette, with
transparent16x16 canvases. Photo references and pixel grids are recorded in each
assets/icons/*-source.json; photographs and game-label artwork are not bundled.

- Game Gear keeps the established cartridge sprite.
- PC Engine uses a portrait9x14 white HuCard, dark upper contact area and gold
  pins along its top edge, following the referenced Fighting Run card.
- Neo Geo Pocket and Color use landscape14x12 shells with upper grip ridges,
  inset labels, lower embossed pads and insertion arrow. Color has a color label.
- Atari2600 uses a portrait12x14 dark shell with raised rim and inset text label.
  Its icon does not imply that an Atari emulator is bundled or hardware-ready.

`python3 tools/generate-cartridge-icons.py --check` verifies RGB555 sources,
pixel grids and tiled headers; `--preview` writes enlarged transparent PNGs under
ignored build/icons. Added sprites reuse the existing native/Game Gear palette.
The read-only ICN1 stream uploads5376 standard or5632 Chis bytes in one halfword
DMA, without restoring icon tiles to main EWRAM or changing native/selector IDs.
The resource packer derives byte counts from the actual arrays.

`bash scripts/test-cartridge-icons.sh` checks colors/transparency, aspect ratios,
extension routing and sprite bounds on all3 board layouts. The ARM resource
oracle compares every packed tile to C initializers and rejects corrupt tails.
Icons are built in and need no SD artwork files. .ngp selects Pocket; .ngc/.ngpc
select Color; .pce selects HuCard; .a26 selects Atari. Emulator compatibility is
separate from file-icon support.
