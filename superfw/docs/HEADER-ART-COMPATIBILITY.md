# ROM-header artwork: AGS history and native record compatibility

Owner Build582 result (2026-11-15): only AGS Aging Cartridge header Title art
renders; ordinary games with No-Intro tags do not. Filename artwork works.

## Historical lead verified

Commit `944ac131d0653b2e4a11e04f368fb00220303be5` (2026-11-07),
“Fix 8.4.1 startup title art compatibility”, changed the DE startup/save-popup
validator in then-`source/save_popup.h` to accept planes0 or1. The current
reference is `ezfomegade/source/save_popup.h`. Its journal records owner hardware
confirmation for `TCHK`, `/SYSTEM/IMGS/T/C/TCHK.bmp`:120x80, top-down16-bit BI_RGB,
54-byte prefix,19254bytes, **planes0**. This was not the `DST1` marker.

The DE browser's `Launcher_LoadThumbnailByGameCode`/`Launcher_ReadThumbnailBytes`
read a fixed54+19200byte record; unmarked artwork ignores header metadata. DST1
opt-in recognition separately checks the BM/signature, offset, DIB, planes1,
120x80 top-down geometry,16bpp and BI_RGB. Its check ignores declared file-size
fields. That distinction explains why sharing one strict BMP parser is not an
accurate port of the native browser reader.

## SuperFW correction

Build582 accepted only exact-size BM legacy code records after strict decoding.
The code-art path now reads a bounded19254byte prefix from any file at least
that long, ignores the opaque legacy prefix, and decodes native top-down GBA
RGB555 pixels. It never allocates or reads the declared full file size. Marked
code art preserves DST1 only when the native DE fixed-layout checks pass;
otherwise it remains opaque. Filename/CUSTOM art retains strict BMP parsing.
Lookup priority and nested/flat code paths remain unchanged.

Regression coverage includes historical TCHK/planes0, ordinary planes1,
nonstandard signature, trailing bytes/declared sizes, truncated records,
valid/invalid markers, and strict filename rejection. No-Intro-shaped filenames
are opened unchanged; the artwork key comes from four bytes at ROM offset0xAC.
Tags are not stripped and cannot change that key. This is adapter/mock and ARM
bus evidence, not an actual FAT directory/card qualification.

The original IMGS archive still returns HTTP403 in this environment. Its actual
bytes have not been inspected. New compatibility follows the verified DE reader
rather than claiming a confirmed diagnosis of every file on the owner's card.
Further hardware observations should identify the numbered build tested.
