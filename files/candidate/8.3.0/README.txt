DS Style Merkin Mix 8.3.0 release candidate

This build includes staged replacement for save files and unified settings. The owner reports that this candidate boots, saves work, and settings remain saved across power cycles.

Host regression tests cover SD write failures and transaction recovery states. Power loss during a FAT directory rename has not been tested on hardware; an ambiguous recovery state preserves the transaction files and can block further save/settings I/O pending manual repair.

Build version: 8.3.0
File: ezkernelnew.bin
