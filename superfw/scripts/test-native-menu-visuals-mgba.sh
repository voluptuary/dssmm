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
for font in ${DSSMM_PREVIEW_FONTS:-1 0}; do
for scene in ${DSSMM_PREVIEW_SCENES:-1 2 3 4 5 6 7 8 9 10 11 12 13 14 20 21 22}; do
 if [[ ( "$scene" = 4 || "$scene" = 13 ) && "$nor_supported" = 0 ]]; then continue; fi
 python3 "$root/scripts/build-native-settings-preview.py" --scene "$scene" --font "$font" --theme "${DSSMM_PREVIEW_THEME:-6}" --topbar "${DSSMM_PREVIEW_TOPBAR:-0}"
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
 python3 - "$out" "$root" "$scene" "$font" "${DSSMM_PREVIEW_THEME:-6}" <<'PY'
from pathlib import Path
import hashlib,struct,sys
out,root=map(Path,sys.argv[1:3]);scene=int(sys.argv[3]);font=int(sys.argv[4]);dark=int(sys.argv[5])==7;sys.path.insert(0,str(root/'tests'))
from png_reader import read_rgb_png
path=max(out.glob('native-settings-*.png'),key=lambda p:p.stat().st_mtime)
w,h,rows=read_rgb_png(path);assert(w,h)==(240,160)
assert all(tuple(rows[y][x*3:x*3+3])==((0,255,0) if x%2==0 else (0,0,255)) for y in range(1,7) for x in range(232,240)),'Native boot/renderer did not complete'
assert not any(tuple(row[x*3:x*3+3])==(255,0,255) for row in rows[16:] for x in range(240)),'Stale carousel pixels remain behind menu'
source=struct.unpack('<38400H',(root/('assets/backgrounds/'+('dark' if dark else 'light')+'-settings.rgb555')).read_bytes())
points=[]
if scene in (10,15):
 points=[]
elif scene in (9,20,21,22):
 points=[(x,y,x) for y in (149,150,151) for x in range(112,224)]
else:
 for y in range(21,152):
  for x in (2,3,234,235,236,237):points.append((x,y,x+12 if x<4 else x-10))
 for y in range(23,150):points.append((4,y,226 if dark else 14))
 for y in (21,22,149,150,151):
  for x in range(4,234):points.append((x,y,(226 if dark else 14) if x==4 and y==149 else 120))
for x,y,sx in points:
 pixel=source[y*240+sx];expected=tuple((((pixel>>b)&31)<<3)|(((pixel>>b)&31)>>2) for b in (0,5,10))
 assert tuple(rows[y][x*3:x*3+3])==expected,f'Scene{scene}: text/selection crossed frame at{x},{y}'
if scene==20 and font==1 and not dark:
 # Independent DE native capture: identical "Interface" glyphs/coordinates.
 ink=bytes(tuple(rows[y][x*3:x*3+3])==(0,0,0) for y in range(24,38) for x in range(23,111))
 assert hashlib.sha256(ink).hexdigest()=='7c10f263ca3e5e76f53f76eec576d96f115e6409b5359f22f235aed9719ff7a8','Settings label baseline differs from DE reference'
if scene==15:
 rgb=lambda x,y:tuple(rows[y][x*3:x*3+3])
 colors={rgb(x,y)for y in (0,15)for x in (*range(178,184),*range(234,240))}
 assert len(colors)==1,'Topbar selector inherited gradient colors'
if scene in (13,14):
 # At 50%, the corrected 200px fill begins at (20,78), including its top/left edges.
 rgb=lambda x,y:tuple(rows[y][x*3:x*3+3])
 fill=rgb(20,78)
 assert all(rgb(x,y)==fill for y in range(78,94) for x in range(20,120)),'Progress fill has gaps'
 assert rgb(120,78)!=fill and rgb(18,78)!=fill and rgb(20,76)!=fill,'Progress fill crossed its outline/percentage'
(out/f'visual-{scene}-font{font}.png').write_bytes(path.read_bytes())
print(f'Native mGBA scene{scene}/font{font}: frame pixels intact; seeded carousel cleared.')
PY
done
done
