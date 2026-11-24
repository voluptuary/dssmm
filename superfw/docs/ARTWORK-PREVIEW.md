# Artwork preview prototype

For end-user pack creation and SD-card setup, see [Make and install artwork packs](ARTWORK-SETUP.md).

In the SD browser, press **Start** to cycle List → title artwork → box artwork → List. The footer shows `Start: Art`. To retain the mode across reboot, use the existing UI settings Save button (`artwork=0|1|2` in /SYSTEM/ui-settings.txt). All three builds (Chis, SuperCard SD and SuperCard Lite) use Title as the default. Select the mode under **UI settings → Artwork** with Left/Right, then choose **Save**. Older unversioned UI settings migrate from the former native defaults; Test 2 settings and newer preserve explicit selections.

Copy DSSCC output into /SYSTEM/IMGS and /SYSTEM/IMGS2, preserving mirrored game folders. For `/Game Boy Advance/P/Game Boy game (USA).gba`, both modes first try the exact ROM filename stem under `CUSTOM`, then in the mirrored folder. Title mode then reads the four-byte GBA game code at header offset `0xAC` and tries the original EZ-FLASH layout, such as `/SYSTEM/IMGS/B/P/BPRE.bmp`, followed by a flat `/SYSTEM/IMGS/BPRE.bmp` fallback. Box mode is filename-only. Filename candidates require the selected dimensions (120×80 title or80×80 box). Game-code art uses the native bounded120×80 record reader described in HEADER-ART-COMPATIBILITY.md; short/failed reads are rejected. No separate manifest is required. UTF-8 names pass through to the native filesystem; actual Unicode/case behavior needs hardware qualification.

The list preview supports exact filename stems, mirrored paths and Title-mode GBA game-code artwork in the native two-letter folder layout with a flat game-code fallback. The horizontal/vertical carousel uses the same artwork loader; its layout/motion and Title decoder/blitter have a synthetic mGBA preview. The candidate now caches selected and adjacent artwork in five SDRAM slots (selected±2), populating one missing image per settled frame. All four synthetic Title/Box and horizontal/vertical initial views pass the isolated mGBA renderer check; real card lookup and transition behavior still require validation. Folder artwork, root-name fallback, extension fallback and NOR/recent previews remain unsupported. Unsupported or missing list artwork leaves the panel blank; Start returns to full-width browsing. This is not complete pack compatibility yet.

## Current owner582 follow-up

Filename artwork renders; header Title renders only AGS/TCHK on the owner's card.
The verified DE history identifies its planes0 field. Current code matches native
opaque record/marker behavior and tests No-Intro paths; original pack bytes remain
unavailable403. Animation toggle/instant mode has been removed, and old config
keys are ignored. Normal carousel transitions remain. Five-slot lookahead loads
visible images first, then outer neighbors without repainting stable pages.
Info's version/hash line is centered. These follow-up changes require a new
numbered hardware test; see the project journals.

## Historical Test4 feedback and next candidate

The owner reports Box art displays slowly and Title ROM-header art still does not display on the SuperChis Prime. This is not evidence that the native code path works on hardware. The reader now loads each bounded BMP in one contiguous FatFS read into a 19,254-byte SDRAM staging buffer, accepts top-down and bottom-up 16-bit BMPs with optional zero size fields, and tries both nested and flat game-code filenames. Host tests cover these cases. Retest on the card is still required; mGBA does not emulate SuperCard SD/Chis storage or its protocol.

The preview uses a 9,600-byte decoded pixel buffer plus a 19,254-byte BMP staging buffer in the existing cartridge SDRAM state, and a small path/cache key in internal RAM. Positive and negative results are cached for the current selection; directory reload, mode change, menu initialization and actions that can reuse RAM invalidate it. No per-frame directory scans or repeated miss probes are performed. The fixed palette is lossy. DST1 transparent pixels retain the panel background; unmarked black stays opaque. Cache and framebuffer writes use halfwords, not byte stores.

Follow-up validation should include physical rendering, selection responsiveness and game/save regression on each supported board. The owner has a Prime v1.0; the SD and Lite variants still need their own hardware checks. No firmware flashing or NOR instructions are implied by this prototype. Preserve the known-good firmware and confirm recovery before installing any future test image.

## Test 1 downloads and first hardware check

The [Test 1 prerelease](https://github.com/voluptuary/dssmm/releases/tag/superfw-test1) has separate `superfw-chis.fw`, `superfw-sd.fw` and `superfw-lite.fw` downloads. For SuperChis Prime v1.0, choose **superfw-chis.fw**. The owner reports that the Chis image chain-loads successfully on SuperChis Prime v1.0 and reports SuperFW 0.22; manual themes are available. The menu remains mostly upstream SuperFW, so it does not yet have the complete DSSMM DS-style layout. This is an initial boot/theme smoke test only; artwork display, gameplay, saves, NOR and installation remain unvalidated. See [upstream installation and recovery guidance](https://superfw.davidgf.net/docs/install/flash/) before considering any other method. Back up saves and current firmware; confirm a recovery route before any internal-flash write. Do not interrupt a flash operation.

Start with booting and SD browsing, theme/accent selection, then Start’s List/title/box modes using one known image. Check marked transparency, opaque black, missing artwork, rapid selection changes and leaving/returning to a directory. Save UI settings and reboot to check persistence. Then test a disposable game/save with backups; record the console model, Prime revision, installed firmware, image used and exact symptoms. Test NOR writes and installation separately after the baseline passes. Bundled GB/GBC emulation is upstream Jagoomba, not MiGoomba.

Candidate decoder compatibility: 16-bit BI_RGB artwork ignores the unused RGB555 high bit, including when interpreting marked transparent black. Host regressions cover this case. Whether the owner's original Title pack uses that bit is unknown; a physical retest remains necessary.

Under **UI settings → Animation**, choose **On** for sliding carousel transitions or **Off** to switch immediately between entries. Save UI settings to retain the choice after reboot. Existing configurations default to On. This switch affects both horizontal and vertical carousel movement; it does not change the separate text/UI animation speed setting.

Animation Off is the fast carousel path: no transition resampling and no neighbor artwork preloads. It draws selected Title/Box art at native size, with system icons for adjacent entries. Vertical layout reserves a native 120×80 image pane and a separate filename column. Thus Off removes scaling work rather than only suppressing visible motion. Actual card responsiveness still needs measurement.

## Native carousel and legacy code-art follow-up (2026-11-15)

Animation Off now loads previous/selected/next cache entries and draws native
120x80 Title or80x80 Box images. Horizontal neighbors extend beyond the left/right
screen edges; vertical neighbors extend beyond the browser body. Cropping keeps
the selected image complete and preserves top/footer UI. It performs no scaling.
New neighbor arrivals invalidate both retained pages. Both animation modes reuse
settled pages; moving AnimationOn redraws. These software changes need physical
performance confirmation. Neighbor loads remain synchronous and one per settled
frame, so slow SD/filesystem operations can still delay input.

The strict DSSCC decoder remains the first choice. For game-code Title fallback
only, an exact19,254-byte BM record can use native EZ fixed-record interpretation:
54-byte prefix followed by120x80 top-down GBA RGB555 pixels. This matches the DE
reader's fixed byte-count/direct-pixel semantics even when legacy BMP metadata
is inconsistent. Header normalization occurs in internal RAM; the cartridge
buffer receives no byte writes. Filename artwork retains strict parsing and DST1
semantics. Original pack download still returns403, so do not call this actual
pack validation or a confirmed fix for the owner's Title artwork.

Animated geometry now preserves DE's horizontal edge positions (-5/185 for
60px neighbors) and vertical next position (Y124 for32px height), rather than
pushing them inward. A clipped scaler samples against the complete original
rectangle, then writes only visible body pixels (Y16..143). Halfword alignment
rounds odd X coordinates down one pixel. Selection remains painted last; missing
art icons are clamped independently. During motion a rectangle may be outside
the body; clipped blitters reject fully invisible rectangles safely. This is
presentation only; no card protocols are imported from DE.

Selected artwork now has one9,600-byte NOLOAD EWRAM hot buffer. The three-image
SDRAM cache remains the backing store; selecting an image copies it once with
DMA, then repeated native/scaled samples read GBA RAM. New cache contents or
artwork invalidation reset the hot identity. The ELF gate checks both byte-safe
metadata placement and the hot image's EWRAM address/size on every board. The
first Chis link uses251,756/257,024EWRAM bytes (97.95%); future code growth is
linker-limited. This is static layout evidence, not hardware speed/stack proof.

BMP staging now reads a word-aligned bulk prefix and an internal-RAM tail,
then halfword-copies the tail to cartridge memory. This prevents newlib memcpy
from emitting invalid cart byte stores at EOF, while preserving contiguous
bulk-sector reads. The tail uses the already-cached final sector. Staging must
be4-byte aligned; odd-length input requires an extra writable padding byte.
See CAROUSEL-ARTWORK-MEMORY.md for the before/after ARM bus reproduction.

## Start geometry and odd VRAM edges

The candidate matches DE's Start thumb rectangle: Title56x37 at(30,48),
Box37x37 at(39,48). Scaled composition masks the leading/trailing halfword lane
for odd x/width, preserving its neighboring pixel and transparency. Hot paired
pixel writes retain halfword access. Source pixels and metadata stay unchanged.

`test-artwork-compositor-arm.py` runs the actual linked function against a
division oracle in36 odd/even/clipped/transparent cases, with VRAM write-size,
edge guards and stack bounds. `test-start-artwork-mgba.sh` seeds read-only
synthetic cache pixels and checks the exact colored rectangle for both fonts
and artwork styles. It tests rendering, not physical SD artwork reads.
