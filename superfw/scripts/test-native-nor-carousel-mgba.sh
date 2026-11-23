#!/usr/bin/env bash
# Real menu renderer with ROM-backed synthetic NOR records and prefilled cache.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$root/build/native-nor-carousel"
emulator="${MGBA_BIN:-/usr/games/mgba}"
for font in ${DSSMM_PREVIEW_FONTS:-1 0}; do
 for scene in ${DSSMM_NOR_SCENES:-16 17 18 19}; do
  python3 "$root/scripts/build-native-settings-preview.py" --scene "$scene" --font "$font" --out "$out"
  SDL_AUDIODRIVER=dummy xvfb-run -a bash -c '
   set -euo pipefail
   cd "$1"
   "$2" -1 native-settings.gba >/dev/null 2>&1 &
   emulator_pid=$!
   finish() { kill "$emulator_pid" 2>/dev/null || true; wait "$emulator_pid" 2>/dev/null || true; }
   trap finish EXIT
   sleep 5
   xdotool key F12
   sleep 1
  ' _ "$out" "$emulator"
  python3 - "$out" "$root" "$scene" "$font" <<'PY'
from pathlib import Path
import shutil,sys
out,root=map(Path,sys.argv[1:3]);scene,font=map(int,sys.argv[3:]);sys.path.insert(0,str(root/'tests'))
from png_reader import read_rgb_png
path=max(out.glob('native-settings-*.png'),key=lambda p:p.stat().st_mtime)
w,h,rows=read_rgb_png(path);assert(w,h)==(240,160)
def pixel(x,y):return tuple(rows[y][x*3:x*3+3])
assert all(pixel(x,y)==((0,255,0) if x%2==0 else (0,0,255)) for y in range(1,7) for x in range(232,240)),'Native NOR fixture failed'
assert not any(pixel(x,y)==(255,0,255) for y in range(16,160) for x in range(240)),'Stale frame pixels remain'
selected=4 if scene in (17,18) else 0
points=((49,40),(49,90),(49,140)) if scene&1 else ((25,67),(120,67),(210,67))
for (x,y),index in zip(points,((selected-1)%5,selected,(selected+1)%5)):
 cube=70+index*20-20
 channels=[((v*31+2)//5) for v in (cube%6,(cube//6)%6,cube//36)]
 expected=tuple((v<<3)|(v>>2) for v in channels)
 assert pixel(x,y)==expected,(scene,font,index,(x,y),pixel(x,y),expected)
shutil.copy2(path,out/f'nor-{scene}-font{font}.png')
print(f'Native NOR carousel scene{scene}/font{font}: focused/neighbors, artwork mode, wrap and frame reuse passed.')
PY
 done
done
