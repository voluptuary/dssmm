# BLUE DS Style target

Active frontend/software donor: this repository's Merkin Mix `ezfode` at `5f700c9a89768309f06993984bb6c3a3190d04ef`. Hardware baseline: [EZ-FLASH blue-kernel](https://github.com/ezflash-team/blue-kernel) at `109f1a050d9c35008379ce61ae40d05dc528c614`. Vendor notices and Apache-2.0 license remain intact. `resources/image.bin` is unchanged, including its FPGA payload and legacy graphics.

Run from the repository root:

```sh
make
bash scripts/verify-blue-port.sh
python3 scripts/pack-blue-kernel.py --code targets/blue/ezbluekernel-development.gba --elf targets/blue/ezbluekernel-development.elf --resource targets/blue/resources/image.bin --output /tmp/ezbluekernel.bin
```

The packer refuses to overwrite an existing output. Install ordinary host dependencies `gcc`, Python 3, `python3-unicorn` and `python3-distutils` as needed, alongside devkitARM/libgba. `make check` runs platform tests; the full verifier also runs 24 shared software/UI suites against private copies of the active BLUE source. The Windows batch file builds raw development output only; it has not been runtime-tested on Windows.

The linker places executable code below 1 MiB, preserves vendor resources at 1 MiB, and places DS Style assets/initializers from offset 0x207000. Every load section is checked against the first metadata block at 0x780000. Packing requires the matching ELF; copying or renaming the raw GBA file does not create a valid update.

This is a hardware-untested experimental port for BLUE / OMEGA Definitive Edition B only. Larger-image updater support, physical boot, saves, NOR writes and plugin execution remain unverified. No Rev. A/B or original OMEGA hardware report qualifies this target. See [the feature matrix and limitations](../../docs/porting/BLUE_STATUS.md).
