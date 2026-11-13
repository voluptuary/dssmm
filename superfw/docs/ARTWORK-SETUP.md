# Make and install SuperFW artwork packs

Current DSSMM SuperFW builds for Chis, SuperCard SD and SuperCard Lite support name-matched artwork made with [DS Style Card Companion (DSSCC)](https://github.com/voluptuary/dsscc), plus EZ-FLASH GBA title images matched from the ROM header. Artwork is shown for files in the SD browser; folder artwork is not supported.

## Build a pack

1. Download the [latest DSSCC release](https://github.com/voluptuary/dsscc/releases/latest) for Windows and open it. Back up your SD card before changing its contents.
2. Open **Custom Pack → Scan SD Card…** (called Custom Art in some versions) and select the **actual SD-card root**, not a ROM subfolder. This lets the artwork follow the same folders as your games.
3. Choose the artwork type you prefer and **Wide and square** to prepare both sizes. Enable **Include Game Boy Advance files** if you want GBA artwork too. This prototype needs name-matched output for GBA as well; the separate traditional full-GBA/code pack is not sufficient.
4. Complete the scan. Check the matched images; use **Set Image…** to supply or replace local artwork where needed. Provider setup and fallback options are explained in [DSSCC’s guide](https://github.com/voluptuary/dsscc#readme).
5. Choose **Mirrored** under **Output Layout** (described as “Mirror ROM folders” in some versions), then **Build Pack** in Custom Pack. Use **For: All** if you want every listed system. The scan offers both sizes: 120×80 title thumbnails go into IMGS and 80×80 box thumbnails into IMGS2.
6. Find the completed pack under `DS Style Card Companion Output/Thumbnail Packs/<timestamp>/`, or the output location shown by your DSSCC version.

For a single custom picture, use **Add ROMs…** or **Add Name…**, set an image and choose its size. With **Custom** output, the artwork name must match the ROM filename without its extension: `Tetris (World).gb` needs `Tetris (World).bmp`. SuperFW checks `CUSTOM` for the exact ROM filename first, then the mirrored ROM folder. In Title mode only, it then tries the GBA internal header title in `CUSTOM` and the mirrored folder, supporting the original EZ-FLASH title images. Build again for the other size if desired. Keep region tags and punctuation identical. This flat layout cannot distinguish two different games with identical filenames; use Mirrored output for those libraries.

## Copy to the SD card

Merge the generated **IMGS** folder into **SYSTEM/IMGS** and **IMGS2** into **SYSTEM/IMGS2**, preserving the subfolders. Do not put another IMGS folder inside SYSTEM/IMGS. Copy the generated BMP files; renaming an ordinary PNG or JPEG to .bmp does not convert it.

For a game stored as `Game Boy Advance/P/Game Boy game (USA).gba`, the SD card can look like this. The DSSCC Box filename matches the ROM filename; the Title filename may either match it or use the ROM header title. If the same image exists in both places, the `CUSTOM` copy wins:

```text
SD card/
├── Game Boy Advance/
│   └── P/
│       └── Game Boy game (USA).gba
└── SYSTEM/
    ├── IMGS/
    │   └── Game Boy Advance/P/Game Boy game (USA).bmp
    └── IMGS2/
        └── Game Boy Advance/P/Game Boy game (USA).bmp
```

For Custom output, place the files at `SYSTEM/IMGS/CUSTOM/Game Boy game (USA).bmp` and `SYSTEM/IMGS2/CUSTOM/Game Boy game (USA).bmp` instead. The `CUSTOM` copy wins over the mirrored copy. If there is no Title image with the ROM filename, Title mode also tries the ROM header title in `CUSTOM` and then its mirrored folder. Creating these artwork folders does not require moving your existing games or saves. SuperFW stores saves and savestates under `SYSTEM` in newer builds; see [the SD-card folder layout](SYSTEM-LAYOUT.md) for legacy-save compatibility details.

## Show artwork in SuperFW

Safely eject the SD card and start any current DSSMM SuperFW build. Title artwork and the DSSMM Light theme are the defaults on Chis, SuperCard SD and SuperCard Lite. An older unversioned UI configuration migrates from the former native defaults; settings saved by Test 2 or newer keep their explicit theme and artwork choices. To choose a mode yourself, open **UI settings → Artwork**, press Left/Right to select **List**, **Title** or **Box**, then choose **Save**. In the SD browser, **Start** also cycles those three modes. Recent and NOR views do not show artwork in this prototype.

If the panel is blank, check the selected mode, image size, exact name, folder paths and that you copied the pack inside SYSTEM. Moving a ROM requires moving its mirrored artwork folder too. Renaming a ROM does not affect an EZ-FLASH header-title fallback, but filename-matched DSSCC artwork must keep the same base name. Leave and re-enter the folder, or cycle the artwork mode, to retry a cached miss. No extension-wide default artwork is supported yet. Generated transparency is supported, but colors are reduced to fit this early menu renderer. Please report visual or scrolling problems when testing on hardware.
