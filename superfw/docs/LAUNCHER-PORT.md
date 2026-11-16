# DS Style launcher port

DSSMM Light/Dark now open a Start screen. Start from a native page opens it;
A/Start opens the selected panel and B returns to the SD browser. The upper
panel opens the most recent game through SuperFW's existing load menu. The
lower panels open SD and NOR (Chis), or SD and Recents (SD/Lite). The small
bottom button opens Settings. D-pad navigation follows DE's four-panel layout.

Settings opens Interface, Games, Hardware, System and Help. These route to the
existing settings/actions; Games and Hardware currently enter different points
in the native settings list. Splitting those into true independent DE-style
categories, matching all dialogs and completing input/visual parity remain work.
B returns to the category page. L/R still reach the native pages. Upstream
non-DSSMM themes keep their icon tabs and Start artwork shortcut. In DSSMM themes,
artwork mode is changed in Interface settings and Start opens the launcher.

The Start backgrounds are exact DE `images/blank/START.h` and
`images/dark/START.h` bitmap assets, credited to DS Style / DSSMM contributors.
Their Apache-2.0 notice is retained in `licenses/dssmm-font-apache-2.0.txt`.
Standalone RGB555 originals live under `assets/backgrounds`. No hardware driver
or private card protocol is imported. `tools/generate-dssmm-background.py`
produces lossless halfword LZ streams (3070 bytes total) and verifies round-trip
pixels. The renderer writes Mode4 halfwords directly, without another screen
buffer. Generic page backgrounds reproduce DE's alternating SD_LIST scanlines.

Palette0..15 belongs to DS backgrounds; roles16..19 and artwork20..235 remain
separate. The Info logo supplies its own palette. Start/settings pages retain
both settled VRAM pages until input/modal/theme changes invalidate them. Carousel
text restores the striped background while keeping the five-image cache.

Cold file/action paths and text-heavy dialogs favor size optimization; browser,
font, scaling and motion loops retain speed optimization. Chis remains close to
its static EWRAM limit; do not add a framebuffer or weaken memory gates.

Host tests compare every background pixel with the source, check bounds, and
exercise extracted production launcher input for all boards. Optional Unicorn
runs the actual ARM renderer and rejects byte/unaligned video writes. mGBA screens
13/14 use actual Start/category renderers and input handlers with fake recent
ROM/artwork and no card/game actions. Preview palettes use the production roles
so backgrounds cannot overwrite text colors. None qualifies real card timing,
save migration, game launching or in-game restoration.

## Name, selectable clock and wrapping

Owner594 feedback adds a solid Start top bar: the first line of
`SYSTEM/NAME.TXT` appears on the left (up to12 complete UTF8 characters; optional
BOM accepted), and a24-hour clock appears on the right. `SYSTEM/NAME` is a
compatibility fallback. The name is read once during startup, not per frame.

Up/Down wraps Last Played → SD/NOR → Settings → Time → Last Played. The middle
row remembers SD or NOR. Left/Right switches those two panels. A/Start on Time
opens an hours/minutes editor; Left/Right chooses the field, Up/Down changes it,
A accepts/saves UI preferences, B cancels. NOR B returns to Start when no modal
is open. Corners are longer/thicker for better visibility.

These cards have no hardware-backed menu RTC. The independent software clock
uses actual GBA frame duration while the menu runs; it does not advance while
powered off or run across games. Before it is set, the bar shows `--:--:--`.
Setting it saves a new starting time, not a claim of battery-backed wall time.
Game RTC defaults and emulation remain separate. No automatic theme scheduling
has been introduced. This explicitly requested software clock supersedes the
initial plan to omit all clock UI on this hardware.

## Read-only background storage

The3076-byte tagged background stream blob now follows the untouched native
font database in the existing14.5..16MiB cartridge SDRAM resource partition.
The native font header/data_size and every original font byte remain identical;
in-game font consumers therefore copy their original data only. Bootloader font
copy/decompression paths retain their native algorithms and addresses, loading
the complete enlarged resource. Build rules generate both normal and extended
packs and check rounded copy sizes against the1.5MiB resource budget. Background
pixels use existing VRAM; no extra framebuffer or image cache was added.

Firmware reads streams with halfword accesses; only native boot copy/unpack
writes the resource area. A missing tag falls back to the striped texture.
Host pixel comparisons and actual ARM embedded/normal/extended resource tests
verify output and reject writes into the read-only font partition. Main Chis
RAM recovers about2.6KiB before the new profile/clock code. Keep packed flash
and GBA RAM gates independent; do not raise hardware limits to fit future UI.

## Full-screen backgrounds after603

Light/Dark horizontal and vertical browser backgrounds and Help panels are
lossless copies of the corresponding DE assets. `assets/backgrounds/parity-sources.json`
records their reference hashes. The generic DS grid repeats the untouched first
48 scanlines of the horizontal asset. All background colors fit the reserved
0–15 palette; UI/artwork palette roles remain separate. DBG3 adds these streams
after the unchanged native font prefix; rounded extended-resource headroom is
499712bytes. Native loader/allocation/unpack paths remain unchanged.

The carousel uses the full screen below the top bar, including y144–159 for the
lower vertical neighbor. The original footer stays in List/native themes; DS
carousel/Info/NOR screens omit its solid bar. Stable carousel pages still reuse
artwork and only restore the caption region. Hardware603 remains frozen while
this later rendering work is validated.

For representative mGBA colors use explicit `PREVIEW_THEME=6` (Light) or7 (Dark)
and `PREVIEW_FONT=1`. Without an explicit theme the fixture intentionally starts
with its schematic palette, which can hide white text on Light panels. Example:
`PREVIEW_THEME=6 PREVIEW_FONT=1 PREVIEW_VERTICAL=1 bash scripts/test-browser-preview-mgba.sh`.
The harness now draws the actual carousel backgrounds with the production layout,
font and compositor, but synthetic games/art and no physical SuperCard storage.
