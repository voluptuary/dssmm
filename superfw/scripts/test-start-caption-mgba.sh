#!/usr/bin/env bash
# Native renderer with ROM-backed recent records and the credited DE glyph oracle.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$root/build/start-caption"
for case_id in 0 1 2 3; do
 args=()
 case "$case_id" in
  1) args=(--recent-title 'Short.gba');;
  2) args=(--recent-title 'Native menu test ROM.gba');;
  3) args=(--recent-title 'First caption row Second caption row Third caption row.gba');;
 esac
 python3 "$root/scripts/build-native-settings-preview.py" --scene 10 --font 1 --out "$out" "${args[@]}"
 SDL_AUDIODRIVER=dummy xvfb-run -a bash -c '
  set -euo pipefail
  cd "$1"
  "$2" -1 native-settings.gba >/dev/null 2>&1 &
  preview_pid=$!
  trap "kill $preview_pid 2>/dev/null || true; wait $preview_pid 2>/dev/null || true" EXIT
  sleep 5
  xdotool key F12
  sleep 1
 ' _ "$out" "${MGBA_BIN:-/usr/games/mgba}"
 python3 - "$root" "$out" "$case_id" <<'PY'
from pathlib import Path
import sys
root,out=map(Path,sys.argv[1:3]);case=int(sys.argv[3]);sys.path.insert(0,str(root/'tests'))
from png_reader import read_rgb_png
p=max(out.glob('native-settings-*.png'),key=lambda p:p.stat().st_mtime_ns)
w,h,rows=read_rgb_png(p);assert (w,h)==(240,160)
assert all(tuple(rows[y][x*3:x*3+3])==((0,255,0) if x%2==0 else (0,0,255)) for y in range(1,7) for x in range(232,240)),'Native boot/renderer did not complete'
lines=[['No recent game'],['Short'],['Native menu test','ROM'],['First caption row','Second caption row','Third caption row']][case]
# Independent DE positions and original row bitmap, without the native FO inset.
top={1:60,2:55,3:49}[len(lines)];glyphs=(root/'assets/fonts/dssmm-ascii-rows.bin').read_bytes();expected=set()
for i,line in enumerate(lines):
 left=150-len(line)*3
 for col,char in enumerate(line):
  for y,bits in enumerate(glyphs[(ord(char)-32)*12:(ord(char)-31)*12]):
   for x in range(6):
    if bits&(128>>x):expected.add((left+col*6+x,top+i*11+y))
actual={(x,y)for y in range(46,86)for x in range(96,212)if tuple(rows[y][x*3:x*3+3])==(0,0,0)}
assert actual==expected,f'Caption{case}: missing{sorted(expected-actual)[:8]}, extra{sorted(actual-expected)[:8]}'
(out/f'caption-{case}.png').write_bytes(p.read_bytes())
print(f'Native Start caption{case}: every text pixel matches the DE glyph/position oracle.')
PY
done
