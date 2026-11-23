#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$root/build/native-start-art"
emulator="${MGBA_BIN:-/usr/games/mgba}"
for font in ${DSSMM_PREVIEW_FONTS:-1 0}; do
 for style in title box; do
  python3 "$root/scripts/build-native-settings-preview.py" --scene 10 --font "$font" --recent-title 'Native artwork.gba' --recent-art "$style" --out "$out"
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
  python3 - "$root" "$out" "$font" "$style" <<'PY'
from pathlib import Path
import sys,shutil
root,out=map(Path,sys.argv[1:3]);font=int(sys.argv[3]);style=sys.argv[4];sys.path.insert(0,str(root/'tests'))
from png_reader import read_rgb_png
path=max(out.glob('native-settings-*.png'),key=lambda p:p.stat().st_mtime)
w,h,rows=read_rgb_png(path);assert(w,h)==(240,160)
def pixel(x,y):return tuple(rows[y][x*3:x*3+3])
assert all(pixel(x,y)==((0,255,0) if x%2==0 else (0,0,255)) for y in range(1,7) for x in range(232,240)),'Native Start did not complete'
# Palette70: cube50, RGB5(12,12,6). Unique synthetic art color.
points={(x,y) for y in range(16,160) for x in range(240) if pixel(x,y)==(99,99,49)}
x0,width=(39,37) if style=='box' else (30,56)
expected={(x,y) for y in range(48,85) for x in range(x0,x0+width)}
assert points==expected,(font,style,len(points),len(expected),list(points^expected)[:5])
shutil.copy2(path,out/f'start-{style}-font{font}.png')
print(f'Native Start {style}/font{font}: exact DE artwork rectangle and retained pixels passed.')
PY
 done
done
