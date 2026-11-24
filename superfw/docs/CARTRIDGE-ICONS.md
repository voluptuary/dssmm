# Native cartridge icons

Game Gear and PC Engine use original DSSMM cartridge artwork matching the
SuperFW icon style. Game Gear keeps its existing pixels; PCE uses a flat HuCard
silhouette, white bevel, maroon label, landscape and contacts. Both have a
transparent16x16 canvas. Sources, palette and credits live under assets/icons.

`python3 tools/generate-cartridge-icons.py --check` checks both RGB555 sources
against their JSON pixel grids and generated tiled headers. `--preview` writes
ignored enlarged transparent PNGs under build/icons. The HuCard reuses existing
native/Game Gear colors; no palette IDs are shifted or added for it.

The read-only ICN1 font-resource tail now contains native tiles followed by
Game Gear and PCE. One halfword DMA uploads the complete4608-byte standard or
4864-byte Chis stream. Moving the extra tiles out of main EWRAM offsets the new
icon's footprint; native tile IDs and selector slot63 remain unchanged. Make
tracks all source/header dependencies to avoid stale incremental resources.

`bash scripts/test-cartridge-icons.sh` checks pixel colors, transparency,
extension lookup and sprite bounds for all three boards. The actual ARM
read-only resource test compares all tiles with the C initializer oracle and
rejects corrupt tags/sizes. The icon is built in; it needs no SD artwork file.
