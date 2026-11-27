# Shared SD image harness

`scripts/verify-shared-sd-image.sh` runs the six DSSMM kernel variants against
the same initial raw SD image. Build the kernels in this checkout first. Pass a
pristine FAT32 or exFAT image as the first argument and the directory containing the
`ezflash-mgba-runner` and `supercard-mgba-runner` executables from
`voluptuary/pogo.gba` as the second argument. The optional third argument is
the results directory.

The script restores the image before each menu and interactive run. Each profile
writes separate images, logs and framebuffer captures. The 1,000-frame menu
capture must contain visible pixels. The 3,200-frame interactive capture can
end in a game or a dialog. The script prints the input and output SHA-256 for
each profile and checks filesystem integrity before and after each run. Keep the input image outside Git and inspect each capture and log
before claiming menu or plugin behavior. Synthetic card responses and emulator
execution do not qualify physical hardware.

The upstream runner source checked for this integration was `pogo.gba` commit
`3e3ac393e6f0ed6edd4de933a57560958cd1daa7`. Its six-profile run script
supplied the profile order and key events. The upstream harness uses C card
models and mGBA bus callbacks. The earlier Lua description in the SuperFW
branch's `docs/HARNESS.md` does not describe that implementation.

The runners build with the mGBA source commit
`c4fe856f2b33d116b8f54267359d4c81653759f6` in
`pogo.gba/harnesses/third_party/mgba`. The packaged mGBA 0.10.1
source lacks `mgba-util/image.h`, which both runners include. Configure the
upstream harness with CMake and build `ezflash-mgba-runner` and
`supercard-mgba-runner`. A fresh devcontainer includes the FAT32/exFAT image
tools. For a minimal FAT32 card image, create a 64 MiB file with `truncate`,
format it with `mkfs.vfat -F 32`, and add `SYSTEM`, `SYSTEM/PLUG`,
`SYSTEM/SAVER`, and `ROMS` using `mmd -i`. Populate the same image with the
desired test plugins, ROMs and artwork before running all six profiles.
