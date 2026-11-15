# SuperFW SD-card folders

Current development builds use one fixed `SYSTEM` layout on SD, Lite and Chis.
Settings, per-game configurations, patch cache, cheats, external emulators,
recent list, flash backups and pending-save/test markers formerly in `.superfw`
now live directly inside `SYSTEM`. Artwork remains in `SYSTEM/IMGS` and
`SYSTEM/IMGS2`. New saves use `SYSTEM/SAVEGAME`; disk states use
`SYSTEM/SAVESTATE`. Directory-choice settings have been removed.

On first boot the firmware moves the contents of `.superfw` into `SYSTEM`,
merging subdirectories. It never replaces an existing destination file. A file
conflict or storage error stops startup before pending SRAM is processed;
resolve duplicates on a computer and retry. Keep the wanted copy of both files
while resolving any conflict. Interrupted moves resume on the next boot. The
legacy directory is removed only after it is empty. No game ROM is moved.

Existing root `SAVEGAME`, `SAVES`, `SAVESTATE`, `SYSTEM/SAVES`, and saves beside
ROMs remain detectable when no preferred SYSTEM save exists, to avoid silently
starting a fresh save. Existing backup/temp save sets remain together. These
compatibility reads are automatic, not user-selectable locations. Move old save
sets into the canonical folders manually after backing them up if desired.

```text
SYSTEM/
  settings.txt, ui-settings.txt, recent.txt
  config/, patches/, cheats/, emulators/
  SAVEGAME/, SAVESTATE/
  IMGS/, IMGS2/
  pending-save.txt, pending-sram-test.txt  (when needed)
  flash_backup-*.bin, superfwlog.txt      (when created)
```

Physical migration, saves and in-game behavior require hardware validation;
host fixture tests verify merging, conflict refusal, retry and resource paths.
