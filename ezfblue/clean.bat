@echo off
setlocal
set "PATH=C:\devkitPro\msys2\usr\bin;C:\devkitPro\devkitARM\bin;%PATH%"
set "DEVKITARM=/c/devkitPro/devkitARM"
set "DEVKITPRO=/c/devkitPro"
set "LIBGBA=/c/devkitPro/libgba"
make clean
