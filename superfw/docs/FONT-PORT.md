# Optional DSSMM font

UI settings → Font selects SuperFW (the default) or DSSMM. Save UI settings to
retain the choice. The choice applies to the browser, settings, system and game
screens and the in-game menu. Non-ASCII characters retain native font fallback.
This is a testable presentation option, not full DE menu parity.

The printable ASCII bitmap comes from DSSMM DE `source/asc126_new.h`
(`ezfomegade/` in this shared tree), generated originally from `asc.dat`.
Credit the DS Style / DSSMM contributors for that system-font asset. Its full
1536-byte source digest is
`f3a11a729c120f176659f6c5eb52f9b2c7b0948d6f176e3751a5d2af69a09240`.
The repository's Apache-2.0 license is retained at
`licenses/dssmm-font-apache-2.0.txt`. No hardware driver is imported.

`assets/fonts/dssmm-ascii-rows.bin` retains the 95 printable glyphs, 12 rows each.
`tools/generate-dssmm-font.py` transposes them to the native column format;
`--check` verifies the generated header. Glyphs advance six pixels except tilde,
which uses seven to retain its final column without an unsafe overhang. Rows
start two pixels into the existing 16-pixel line. Native fallback retains its
own metrics. Clipping, scrolling and measurement use the selected font.

Settings version7 stores `font=0|1`; old configurations retain native default.
In-game propagation uses reserved word5 of the existing eight-word palette
handoff, leaving payload offsets unchanged. Unknown values select native.
Physical in-game rendering/restoration still requires card testing.

The host regression compares every source pixel at even and odd destinations,
checks canaries, metrics, clipped scrolling, Unicode fallback and restoration.
mGBA fixtures test presentation only; they do not emulate SuperCard protocols.
