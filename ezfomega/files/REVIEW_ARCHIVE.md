# Archived Reviews and Candidate Notes

This file consolidates the historical review notes and candidate summaries that were superseded by the integrated code and current audit ledgers. The chronological record and detailed validation history remain in `../HANDOFF.md` and `../PROGRESS.md`.

## Carousel performance

- The owner observed gradual carousel slowdown as folder size increased, noticeable around 60 titles; this was never established as a hard threshold.
- Source review found bounded rendering work but directory/path lookup and synchronous artwork loading that can scale with folder size. No hardware timing result was inferred from host operation counts.
- The CUSTOM manifest change sorted the bounded manifest and used binary search. Host differential/operation-count tests validated lookup membership and scaling; it did not prove a hardware speedup.
- The later FAT/FAT32 header-art fast-open path records short-entry offsets beside indexed file records, validates short name and size, then falls back to normal path open on mismatch or unsupported filesystem mode.

## Upstream UI integration

- The UI port was selective: Auto theme, grouped view options, SYSTEM browser, wrap settings, navigation, start-selection motion, popup/launch presentation and related settings behavior were integrated.
- Local Clean list and Remember cheats options, CUSTOM Only behavior, fork-specific artwork handling and other existing protections were retained. Historical RC1/RC2/RC3 candidate details are in the audit ledgers.
- Host regression coverage exercises defaults/aliases, navigation bounds, Auto theme hours, SYSTEM-browser restoration/failure/reentry and start-motion retargeting. The current integrated code is on `main`.

## Presentation and artwork

- The presentation review covered startup effects, sound/popup behavior, launch indicator, theme transition, Start markers and save-data popup artwork.
- The border silhouette experiment was rolled back after owner hardware feedback; existing rectangular/rounded borders and marked transparency behavior were retained.
- Save-popup artwork now follows the configured Title/Box style and validates BMP dimensions/format. Hardware reports and follow-up fixes are recorded chronologically in the audit ledgers.

## Skiller cheat engine

- The review recommended against wholesale engine replacement. A staged compiler/interpreter port would require differential tests and target hardware validation; the legacy engine remains the default.

## Artifact policy

- Superseded release/test binaries were removed during repository cleanup. The active release candidate is `candidate/8.3.0/ezkernelnew.bin`; it is refreshed from the final 8.3.0 source build. Superseded carousel and upstream UI raw logs were retired after their findings and validation summaries were consolidated in this archive and the audit ledgers. Candidate hashes, build sizes and validation outcomes remain recorded in `../PROGRESS.md`.


## Retired Claude patch archive

- `files/claude-audit.tar.gz` was a historical review bundle. Its patch and finding disposition was reviewed and recorded in the audit ledgers; the integrated artwork optimization and applicable fixes remain in source and current regressions. The user authorized retiring the archive after integration. No active build, test or release workflow depends on it.
