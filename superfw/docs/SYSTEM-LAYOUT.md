# SuperFW SD-card folders

Current development builds use one fixed `SYSTEM` layout on SD, Lite and Chis.
Settings, per-game configurations, patch cache, cheats, external emulators,
recent list, flash backups and pending-save/test markers formerly in `.superfw`
now live directly inside `SYSTEM`. Artwork remains in `SYSTEM/IMGS` and
`SYSTEM/IMGS2`. New saves use `SYSTEM/SAVER`; disk states use
`SYSTEM/SAVESTATE`. Directory-choice settings have been removed.

On first boot the firmware moves the contents of `.superfw` into `SYSTEM`,
merging subdirectories. It never replaces an existing destination file. A file
conflict or storage error stops startup before pending SRAM is processed;
resolve duplicates on a computer and retry. Keep the wanted copy of both files
while resolving any conflict. Interrupted moves resume on the next boot. The
legacy directory is removed only after it is empty. No game ROM is moved.

Existing `SYSTEM/SAVEGAME`, root `SAVEGAME`, `SAVES`, `SAVESTATE`, `SYSTEM/SAVES`, and saves beside
ROMs remain detectable when no preferred SYSTEM save exists, to avoid silently
starting a fresh save. Existing backup/temp save sets remain together. These
compatibility reads are automatic, not user-selectable locations. Move old save
sets into the canonical folders manually after backing them up if desired.

```text
SYSTEM/
  SETTINGS.TXT, RECENT.TXT, SUPERFW-RECENT.TXT
  config/, patches/, cheats/, PLUG/
  SAVER/, SAVESTATE/
  IMGS/, IMGS2/
  pending-save.txt, pending-sram-test.txt  (when needed)
  flash_backup-*.bin, superfwlog.txt      (when created)
```

Physical migration, saves and in-game behavior require hardware validation;
host fixture tests verify merging, conflict refusal, retry and resource paths.

Settings now share the DE `SETTINGS.TXT` key names first, followed by namespaced
`SuperFW.*` values. Unknown DE/SuperFW values are retained, and the legacy
UI-SETTINGS.TXT sidecar is removed only after a successful merged save. Settings
save when a menu/submenu is exited; failures leave the menu and edited values
available for retry. Unsupported hardware options remain platform-specific;
copying SYSTEM cannot make EZ-FLASH drivers, patch caches or native savestates
interchangeable with SuperCard formats.

RECENT.TXT contains up to ten newest SD paths in the DE-compatible format.
SUPERFW-RECENT.TXT retains the larger native history and NOR entries. Selection
records the game before launch even when the Recent menu is hidden. When another
kernel changes the shared SD order, SuperFW merges that order ahead of its older
history. If the same SD title remains first, private NOR ordering is retained;
that format cannot distinguish an external reselection of the same first title.

SYSTEM/emulators is safely merged into PLUG without overwriting files. See
PLUGIN-COMPATIBILITY.md for extension-named plugins and hardware limits.
