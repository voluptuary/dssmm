# Native menu showcase captures

The README GIFs show actual EZ-FLASH DE and SuperFW Chis renderers in stock
SDL mGBA0.10.1. They are not simulated web UIs or interpolated animation.
Original fictional cover/title samples are in demo-art/; the built-in image_gen
prompts and image fingerprints are retained there. These are illustrative
assets, not actual games. Hardware timing/storage behavior is not qualified.

The universal record-menu-preview.py and assemble-menu-previews.py tools use
Pillow, a private Xvfb display/config and real D-pad input. Capture metadata
records native ROM/image fingerprints and timing. GIFs use2x nearest-neighbor
presentation and a shared256-color palette. No BIOS or executable fixture ROM
is included in the public documentation assets.

From repository root with the usual devkitARM environment:

```sh
python3 ezfomegade/scripts/build-native-menu-preview.py --scene 7 --art-dir docs/media/demo-art
python3 scripts/record-menu-preview.py --rom ezfomegade/build/native-preview.gba --out /tmp/ezflash-horizontal.gif --keys Right,Right,Left
# Repeat with scene8 and --art-style box for the vertical clip.
make superfw BOARD=chis
python3 superfw/scripts/build-native-settings-preview.py --scene 16 --font 1 --reference --showcase --art-dir docs/media/demo-art --out superfw/build/readme-horizontal
python3 scripts/record-menu-preview.py --rom superfw/build/readme-horizontal/native-settings.gba --out /tmp/superfw-horizontal.gif --keys Right,Right,Left
# Repeat with scene19 and Down,Down,Up for the vertical Box clip.
python3 scripts/assemble-menu-previews.py --out /tmp/carousels.gif /tmp/horizontal.gif /tmp/vertical.gif
```

Static menu montages use native scene captures with1.4s holds. SuperFW includes
Start, Interface, Loading and Patching options; DE includes Start and grouped
settings. Generated binaries/screenshots remain in ignored project build dirs;
only the final GIFs, sample PNGs, prompts and provenance are tracked here.
