#!/usr/bin/env bash
# Pixel-exact mGBA test of the __GBA__ hardware/DMA path; BIOS file not needed.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$(mktemp -d)"
trap 'rm -rf "$out"' EXIT
emulator="${MGBA_BIN:-/usr/games/mgba}"
"$emulator" --version
source="${DSSMM_BG_SOURCE:-$root/src/dssmm_background.c}"
arm-none-eabi-gcc -O2 -mcpu=arm7tdmi -mthumb -D__GBA__ -ffunction-sections -fdata-sections \
 -I "$root/src" -specs=gba.specs "$root/tests/background-preview-rom.c" "$source" \
 "$root/src/gbahw.c" -Wl,--gc-sections -o "$out/background-preview.elf"
arm-none-eabi-objcopy -O binary "$out/background-preview.elf" "$out/background-preview.gba"
python3 - "$out/background-preview.gba" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1]);data=bytearray(p.read_bytes());data[0xbd]=(-0x19-sum(data[0xa0:0xbd]))&255;p.write_bytes(data)
PY
SDL_AUDIODRIVER=dummy xvfb-run -a bash -c '
 set -euo pipefail
 cd "$1"
 "$2" -1 background-preview.gba >/dev/null 2>&1 & emulator_pid=$!
 finish(){ kill "$emulator_pid" 2>/dev/null || true; wait "$emulator_pid" 2>/dev/null || true; }
 trap finish EXIT
 sleep 5
 for scene in $(seq 0 9); do
  xdotool key F12
  sleep 0.2
  xdotool keydown Right; sleep 0.08; xdotool keyup Right
  sleep 0.2
 done
' bash "$out" "$emulator"
python3 - "$root" "$out" <<'PY'
from pathlib import Path
import struct,sys
root,out=map(Path,sys.argv[1:]);sys.path.insert(0,str(root/'tests'))
from png_reader import read_rgb_png
shots=sorted(out.glob('background-preview-*.png'),key=lambda p:p.stat().st_mtime_ns)
assert len(shots)==10,f'Expected10 frames, got{len(shots)}'
for scene,image in enumerate(shots):
 theme='dark' if scene>=5 else 'light';kind=('start','settings','horizontal','vertical','grid')[scene%5]
 w,h,rows=read_rgb_png(image);assert (w,h)==(240,160)
 expected=struct.unpack('<38400H',(root/'assets/backgrounds'/f'{theme}-{kind}.rgb555').read_bytes())
 for i,pixel in enumerate(expected):
  channels=[(pixel>>shift)&31 for shift in (0,5,10)]
  rgb=tuple((v<<3)|(v>>2) for v in channels)
  actual=tuple(rows[i//240][i%240*3:i%240*3+3])
  assert actual==rgb,f'{theme}/{kind} pixel{i}: {actual}!={rgb}'
 print(f'mGBA {theme}/{kind}: all38400 source pixels match; padding guard intact.')
PY
