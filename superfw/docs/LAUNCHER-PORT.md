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
