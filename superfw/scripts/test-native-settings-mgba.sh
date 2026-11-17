#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$root/build/native-settings"
emulator="${MGBA_BIN:-/usr/games/mgba}"
python3 "$root/scripts/build-native-settings-preview.py"
rm -f "$out/native-settings.sav" "$out"/native-settings-*.png
for boot in 1 2; do
 SDL_AUDIODRIVER=dummy xvfb-run -a bash -c '
  set -euo pipefail
  cd "$1"
  "$2" -1 native-settings.gba >/dev/null 2>&1 &
  emulator_pid=$!
  finish() { kill "$emulator_pid" 2>/dev/null || true; wait "$emulator_pid" 2>/dev/null || true; }
  trap finish EXIT
  sleep 6
  if [ "$3" = 2 ]; then
    for n in 1 2 3 4 5 6 7; do
      xdotool keydown Down
      sleep 0.1
      xdotool keyup Down
      sleep 0.2
    done
  fi
  xdotool key F12
  sleep 1
 ' _ "$out" "$emulator" "$boot"
 python3 - "$out" "$root/tests" "$boot" <<'PY'
from pathlib import Path
import sys
out=Path(sys.argv[1]);sys.path.insert(0,sys.argv[2])
from png_reader import read_rgb_png
screens=sorted(out.glob('native-settings-*.png'),key=lambda p:p.stat().st_mtime);assert screens,'mGBA screenshot missing'
path=screens[-1];w,h,rows=read_rgb_png(path);assert(w,h)==(240,160)
start=232 if sys.argv[3]=='1' else 224
assert all(tuple(rows[y][x*3:x*3+3])==(0,255,0) for y in range(1,7) for x in range(start,240)), 'Native settings save/boot validation did not reach success marker'
(out/('boot-'+sys.argv[3]+'.png')).write_bytes(path.read_bytes())
if sys.argv[3]=='2':
 import struct
 original=struct.unpack('<38400H',(Path(sys.argv[2]).parent/'assets/backgrounds/light-settings.rgb555').read_bytes())
 for y in range(149,152):
  for x in range(112,224):
   pixel=original[y*240+x];rgb=tuple(((pixel>>b)&31)*8+(((pixel>>b)&31)>>2) for b in (0,5,10))
   assert tuple(rows[y][x*3:x*3+3])==rgb,'Selection/text crossed the bottom frame'
assert (out/'native-settings.sav').exists(),'mGBA virtual SD save was not persisted'
print('mGBA native settings boot'+sys.argv[3]+' passed (real linked ARM menu/settings/FatFs, virtual SRAM SD).')
PY
done
