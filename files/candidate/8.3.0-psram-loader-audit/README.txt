DS Style Merkin Mix 8.3.0 PSRAM loader hardware-test build

This build includes bounded PSRAM loading for GBA games and emulator plugins:
file sizes and combined placements are checked, failed or short reads stop the
launch, and DMA writes are split at the existing PSRAM window boundaries.

Software validation passed: production build, all 33 host test scripts,
browser installer scenarios, JavaScript syntax checks, whitespace checks,
linker layout guards, and Cppcheck. Physical PSRAM page-selection behavior has
not yet been verified on hardware. Please test on supported EZ-Flash Omega
Definitive Edition Rev. A and Rev. B hardware and report boot behavior for
direct GBA, addon/patch-enabled GBA, and supported emulator-plugin launches.

This is a hardware-test candidate, not a new published release. The displayed
kernel version remains 8.3.0.

Build version: 8.3.0
Source commit: b2c454c7279341fe898946137a42e6085cd8c94a
File: ezkernelnew.bin
Size: 2014664 bytes
SHA-256: 9a5e78f69fdddc4c7556b0521702765b0b2a99a7e87e76ba0980b7771eb9952c
