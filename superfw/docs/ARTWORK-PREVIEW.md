# Artwork preview prototype

For end-user pack creation and SD-card setup, see [Make and install artwork packs](ARTWORK-SETUP.md).

In the SD browser, press **Start** to cycle List → title artwork → box artwork → List. The footer shows `Start: Art`. To retain the mode across reboot, use the existing UI settings Save button (`artwork=0|1|2` in /.superfw/ui-settings.txt). List remains the default.

Copy DSSCC output into the same /SYSTEM/IMGS and /SYSTEM/IMGS2 layout used by DSSMM. For /Game Boy/Tetris.gbc, title lookup tries /SYSTEM/IMGS/Game Boy/Tetris.bmp and then /SYSTEM/IMGS/CUSTOM/Tetris.bmp. Box mode uses IMGS2. Each candidate must have the selected dimensions (120×80 title or 80×80 box); malformed, short or failed reads are rejected. No separate manifest is required. UTF-8 names pass through to the native filesystem; actual Unicode/case behavior needs hardware qualification.

This first preview supports exact filename stems and mirrored paths for files. Folder artwork, GBA header-code packs, root-name fallback, extension fallback, NOR/recent previews and carousels remain pending. Unsupported or missing artwork leaves the panel blank; Start returns to full-width browsing. It is not complete pack compatibility yet.

The preview uses 9600 bytes in the existing cartridge SDRAM state and a small path/cache key in internal RAM. Positive and negative results are cached for the current selection; directory reload, mode change, menu initialization and actions that can reuse RAM invalidate it. No per-frame directory scans or repeated miss probes are performed. The fixed palette is lossy. DST1 transparent pixels retain the panel background; unmarked black stays opaque. Cache and framebuffer writes use halfwords, not byte stores.

Validation must still include physical rendering, selection responsiveness and game/save regression on the owner’s Prime v1.0. No firmware flashing or NOR instructions are implied by this prototype. Preserve the known-good firmware and confirm recovery before installing any future test image.

## Test 1 downloads and first hardware check

The [Test 1 prerelease](https://github.com/voluptuary/dssmm/releases/tag/superfw-test1) has separate `superfw-chis.fw`, `superfw-sd.fw` and `superfw-lite.fw` downloads. For SuperChis Prime v1.0, choose **superfw-chis.fw**. This first build has not booted on the owner's card: a chain-load attempt showed a loading bar and then returned to the default loader. Do not treat chain-loading as a working test method for this build. See [upstream installation and recovery guidance](https://superfw.davidgf.net/docs/install/flash/) before considering any other method. Back up saves and current firmware; confirm a recovery route before any internal-flash write. Do not interrupt a flash operation.

Start with booting and SD browsing, theme/accent selection, then Start’s List/title/box modes using one known image. Check marked transparency, opaque black, missing artwork, rapid selection changes and leaving/returning to a directory. Save UI settings and reboot to check persistence. Then test a disposable game/save with backups; record the console model, Prime revision, installed firmware, image used and exact symptoms. Test NOR writes and installation separately after the baseline passes. Bundled GB/GBC emulation is upstream Jagoomba, not MiGoomba.
