# Native NOR carousel checks

Chis NOR browsing reuses the SD horizontal/vertical renderer and its five-image
cache. List mode retains capacity and management controls. Carousel arrows move
one title, respect the existing Wrap around setting and use the same motion and
stable-frame reuse as SD. Launch/delete operations retain their original paths.
No second artwork pool or hardware protocol is introduced.

NOR artwork checks CUSTOM and mirrored filename art first. Title then uses the
stored NOR game code, without reopening the original SD ROM. Start's Last Played
uses the same lookup order. Box never falls back to ROM-code Title pictures.

`bash scripts/test-carousel-source.sh` extracts the production source helpers and
checks SD/NOR counts, selection, paths, icons, directory routing and artwork
priority in NOR-enabled and non-NOR builds. Existing cache/layout tests cover
slot protection and small-list/wrap bounds; recent-artwork tests cover Start.

`bash scripts/test-native-nor-carousel-mgba.sh` renders scenes16–19 from the actual
linked Chis ELF in stock SDL mGBA0.10.1. Both fonts exercise horizontal/vertical
Title/Box views, including first-to-last input through the actual key handler.
Five synthetic NOR records and flat-color decoded images are embedded in the
fixture ROM. Only the corresponding internal-RAM cache descriptors are seeded.
This avoids treating mGBA's read-only cartridge ROM as writable SuperCard SDRAM.
The test samples focused/previous/next image colors, checks completion markers
and rejects stale frame pixels. It also exercises stable reuse of both pages.

This fixture tests the renderer and navigation. It does not emulate NOR flash,
physical SD artwork reads, gameplay, saves or SuperCard timing. Decoder/FatFs
fault and cartridge-bus checks remain separate. No BIOS, commercial game,
emulator patch or new dependency is required; fixture sources and runner are
tracked and generated files remain under build/native-nor-carousel.
