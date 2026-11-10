@echo off
setlocal
set "PATH=C:\devkitPro\msys2\usr\bin;C:\devkitPro\devkitARM\bin;%PATH%"
set "DEVKITARM=/c/devkitPro/devkitARM"
set "DEVKITPRO=/c/devkitPro"
set "LIBGBA=/c/devkitPro/libgba"
make
if errorlevel 1 exit /b 1
echo BLUE development raw image only. Not a standalone update or hardware-tested build.
echo See docs/porting/BLUE_STATUS.md for resource packaging and limitations.
