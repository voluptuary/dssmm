# Native mGBA visual reference

Run `bash scripts/test-native-menu-preview-mgba.sh` with the usual devkitARM
build environment. Stock SDL mGBA0.10.1, Xvfb and xdotool from the devcontainer
are sufficient; no BIOS or emulator modifications are required.

The opt-in wrapper compiles this project's own kernel translation unit and
assets. It supplies a deterministic clock, name, read-only synthetic directory
and decoded artwork cache, and bypasses hardware initialization/actions. Nine
separate boots capture Start, Settings, Interface, Games, Hardware, View, Info,
horizontal and vertical carousel screens. Screenshots are ignored files under
`build/native-scene-0.png` through `native-scene-8.png`. The SRAM signature must
identify the requested completed native renderer on every boot. PNG dimensions
and nonblank size are checked; these are smoke gates, not pixel-perfect parity
assertions. Inspect the captures or compare with the shared root HTML tool.

`python3 scripts/build-native-menu-preview.py --scene N` builds one scene; L/R
can cycle screens and Up/Down changes selection for manual inspection. Native
production menu actions cannot be selected in this fixture. Info uses fixture
text with the actual native text-page renderer. Test ROMs pad a read-only table
at the native PSRAM directory address; they are visual ROMs, never installable
firmware. No card driver or asset is imported from another hardware project.

Production builds omit PREVIEW_SCENE and keep their existing build/layout gates.
Visual tests establish renderer behavior under synthetic state, not SD, NOR,
flash/save timings, game launch or physical-card qualification. Compare commits
by retaining their PNG/JSON outputs; metadata records source commit, dirtiness,
fixture limits and ROM SHA-256. Generated outputs can be rebuilt and must not be
committed as firmware or release assets.
