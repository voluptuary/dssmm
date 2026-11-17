# Native settings crash regression

The owner withdrew Test6/Build640 after Interface changes froze a SuperChis.
The host settings/FatFs tests passed, but replaying the actual linked ARM firmware
reproduced an invalid string-pointer read during settings serialization.

With devkitARM GCC16.1.0 and IPA register allocation enabled, the caller retained
the local View-mode string-table address in r1 across `shared_current()`. The
Thumb1 callee returned with `pop {r1}; bx r1`, overwriting that register. The caller
then indexed the return instruction stream as a string-pointer table. The tested
image passed0xbff9b3f7 to `%s` and faulted in nanoprintf. Another call in
`settings_owned()` also retained a pointer in r1, affecting preservation of prior
settings. This is an ARM code-generation/ABI issue, not evidence of invalid
EZ-FLASH setting names or a stack overflow.

SuperFW ARM firmware/payload builds now disable IPA register allocation with
`-fno-ipa-ra`. Save-on-change was removed; the original requested save-on-exit
remains. The corrected native replay saves and reloads preferences through the
actual FatFs code, with a measured6760-byte stack peak in the initial Chis case.
Hardware budgets and the16KiB minimum stack reserve are unchanged.

`test-native-settings-arm.py` loads the actual ELF and replaces only SD sector
reads/writes with a virtual FAT16 disk. It executes production formatting,
parsing, transactions and FatFs, and checks values and stack bounds. The
unmodified compiler control reproduces the invalid pointer; the workaround
passes. Every board build runs this regression.

`test-native-settings-mgba.sh` additionally runs the linked firmware's real menu
and settings in stock mGBA, using SRAM-backed virtual SD storage. Actual menu
key handlers change preferences and save on exit. A second emulator process
restores those preferences from the persisted save file. It also checks the
last-row selection against the source panel frame. See NATIVE-SETTINGS-PREVIEW.md.

These tests qualify the tested ARM/UI/filesystem paths. They do not emulate
SuperCard electrical protocols, SD timing, real power loss or physical hardware.
The next hardware-test candidate must be identified separately; Build640 remains
withdrawn and must not be republished or retagged.
