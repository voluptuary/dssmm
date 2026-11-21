#!/usr/bin/env bash
# Render actual optimized firmware screens; seed stale art only in the back page.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$root/build/native-settings"
emulator="${MGBA_BIN:-/usr/games/mgba}"
if [[ ! -f "$out/native-settings.sav" ]]; then
 bash "$root/scripts/test-native-settings-mgba.sh"
fi
python3 "$root/scripts/build-native-settings-preview.py" --scene 0 >/dev/null
nor_supported=$(python3 -c 'import json,sys;print(int(json.load(open(sys.argv[1]))["nor_supported"]))' "$out/fixture.json")
for font in 1 0; do
for scene in 1 2 3 4 5 6 7 8 9 10 11 12 13; do
 if [[ ( "$scene" = 4 || "$scene" = 13 ) && "$nor_supported" = 0 ]]; then continue; fi
 python3 "$root/scripts/build-native-settings-preview.py" --scene "$scene" --font "$font"
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
import struct,sys
out,root=map(Path,sys.argv[1:3]);scene=int(sys.argv[3]);font=int(sys.argv[4]);sys.path.insert(0,str(root/'tests'))
from png_reader import read_rgb_png
path=max(out.glob('native-settings-*.png'),key=lambda p:p.stat().st_mtime)
w,h,rows=read_rgb_png(path);assert(w,h)==(240,160)
assert all(tuple(rows[y][x*3:x*3+3])==((0,255,0) if x%2==0 else (0,0,255)) for y in range(1,7) for x in range(232,240)),'Native boot/renderer did not complete'
assert not any(tuple(row[x*3:x*3+3])==(255,0,255) for row in rows[16:] for x in range(240)),'Stale carousel pixels remain behind menu'
source=struct.unpack('<38400H',(root/'assets/backgrounds/light-settings.rgb555').read_bytes())
points=[]
if scene==10:
 points=[]
elif scene==9:
 points=[(x,y,x) for y in (149,150,151) for x in range(112,224)]
else:
 for y in range(21,152):
  for x in (2,3,234,235,236,237):points.append((x,y,x+12 if x<4 else x-10))
 for y in (21,22,149,150,151):
  for x in range(4,234):points.append((x,y,120))
for x,y,sx in points:
 pixel=source[y*240+sx];expected=tuple((((pixel>>b)&31)<<3)|(((pixel>>b)&31)>>2) for b in (0,5,10))
 assert tuple(rows[y][x*3:x*3+3])==expected,f'Scene{scene}: text/selection crossed frame at{x},{y}'
(out/f'visual-{scene}-font{font}.png').write_bytes(path.read_bytes())
print(f'Native mGBA scene{scene}/font{font}: frame pixels intact; seeded carousel cleared.')
PY
done
done
