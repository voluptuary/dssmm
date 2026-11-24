# Game Gear files and save names

`.gg` and `.sms` both select the `sms` plugin in `SYSTEM/PLUG`, trying BIN, GBA,
MB and MBZ in the usual order. Without that plugin, both retain the native
SuperFW SMS emulator fallback list. Game Gear launches keep their original `.gg`
filename and the native GG-specific wrapper flags; they are not renamed to SMS.

Game Gear entries use original DSSMM cartridge pixel art matching SuperFW's
small beveled cartridge icons, replacing the earlier imported DE handheld-system
picture. The dark cartridge, recessed label and GG marking fit a transparent
16×16 sprite. Editable pixel rows/palette and credits are recorded in
`assets/icons/gamegear-source.json`; `gamegear.png` is a transparent preview and
`gamegear.rgb555` supplies the firmware generator. The new art is GPL-3.0-or-later;
upstream SuperFW icon authors retain their credits. Native sprite/palette IDs
remain unchanged, and selector palette slot255 stays reserved.

The native save-name routine strips an extension by finding the final dot, so
both two-character GB/GG and three-character extensions are supported. Focused
all-board tests cover `.gb`, `.gg`, uppercase extensions, NoIntro tags, version
dots, backup/temp and legacy locations, savestate stems and oversized names.
They also caught and fixed a separate double-stripping bug: after removing the
ROM extension, code replaced an extension again, cutting real dots from the
stem. Save names now append `.sav` once and refuse overflow. For example,
`Sonic.v1.2 (USA).gg` uses `SYSTEM/SAVER/Sonic.v1.2 (USA).sav`.

Older builds may have created shortened saves for dotted basenames. When no correctly named save exists, the loader recognizes the exact historical
shortened-name rule in current/legacy locations, preserving prior save access.
It does not move or rename those files. A correctly named save always takes
priority. Back up and rename a legacy save to the complete basename when
transferring it; this fallback cannot repair old collisions between names sharing
a prefix. This is filename-helper
validation, not physical emulator/save-cycle qualification or interchangeability
of differing emulator state formats.

Run `bash scripts/test-cartridge-icons.sh`, `bash scripts/test-pogo-plugins.sh` and
`bash scripts/test-theme-settings.sh`. The icon test checks every cartridge pixel,
transparent padding, palette/VRAM budgets and actual extension-icon lookup on all
three variants. Plugin tests check shared GG/SMS resolution and native GG flags.
