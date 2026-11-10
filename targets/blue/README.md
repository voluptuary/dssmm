# BLUE hardware baseline

Imported from https://github.com/ezflash-team/blue-kernel at 109f1a050d9c35008379ce61ae40d05dc528c614. Vendor notices and Apache-2.0 license are retained. Resource image.bin is byte-identical; the Windows executable is not needed or bundled.

This builds the vendor classic menu as the initial hardware baseline, not the DS Style launcher yet. Root source/ and images/ retain the Merkin Mix donor pending integration; they are excluded from the active build. Modern-toolchain changes add missing prototypes, correct the HZK12 filename case, and make the metadata writer's byte/word input contract explicit. Flash commands, firmware bytes, save layout, and reset assembly are unchanged at this checkpoint.

Run make from the repository root. Raw output is ezbluekernel-development.gba here. It requires the separately placed vendor resources and must not be booted/flashed on its own. scripts/pack-blue-kernel.py reproduces the statically inspected vendor packaging recipe with bounds/resource verification. No installable public candidate or hardware qualification is provided. See ../../docs/porting/BLUE_STATUS.md.
