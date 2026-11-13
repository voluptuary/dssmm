# SuperFW SD-card folders

Newer DSSMM SuperFW builds group user-facing system data under `SYSTEM`, alongside the DSSCC artwork folders. `/.superfw` remains the private SuperFW configuration/data directory.

```text
SD card/
├── SYSTEM/
│   ├── IMGS/              # title artwork
│   ├── IMGS2/             # box artwork
│   ├── SAVEGAME/          # default GBA save location
│   ├── SAVES/             # alternate GBA save location
│   ├── SAVESTATE/         # disk savestates
│   └── superfwlog.txt     # optional diagnostic log
├── .superfw/              # private settings, cache, recent list and tools
└── Games/                 # your game folders stay where they are
```

The selected save policy chooses `SYSTEM/SAVEGAME` or `SYSTEM/SAVES`. Savestates default to `SYSTEM/SAVESTATE`; the alternate `.superfw/savestate` option remains available. ROMs are not moved.

Older cards may still have saves in root-level `/SAVEGAME/` or `/SAVES/`, and states in `/SAVESTATE/`. SuperFW checks those legacy locations when no matching SYSTEM file exists and continues using an existing legacy save set, including its backup files. It does not delete or automatically move those files. When the save-location setting is one of the system folders, games with no legacy save use the SYSTEM paths; the optional “next to ROM” policy remains unchanged. Keep a backup before manually moving saves, and verify in-game save contents before deleting old copies.

The first Chis build with the new artwork default migrates an old saved List default to Title. You can change the mode at **UI settings → Artwork** and save it; `Start` in the SD browser also cycles List, Title and Box.
