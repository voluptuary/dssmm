#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
python3 "$root/scripts/build-migoomba-preview.py"
out="$root/build/migoomba-preview"
for system in gb gbc; do
 python3 - "$out" "$system" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1])/(sys.argv[2]+'.sav')
if p.exists():p.unlink()
PY
 SDL_AUDIODRIVER=dummy xvfb-run -a bash -c '
  set -euo pipefail
  cd "$1"
  "${MGBA_BIN:-/usr/games/mgba}" -1 "$2.gba" >/dev/null 2>&1 &
  preview_pid=$!
  trap "kill $preview_pid 2>/dev/null || true; wait $preview_pid 2>/dev/null || true" EXIT
  sleep 5
  xdotool key F12
  sleep 1
 ' _ "$out" "$system"
 python3 - "$root" "$out" "$system" <<'PY'
from pathlib import Path
import sys
root,out=map(Path,sys.argv[1:3]);system=sys.argv[3];sys.path.insert(0,str(root/'tests'))
from png_reader import read_rgb_png
path=max(out.glob(system+'-*.png'),key=lambda p:p.stat().st_mtime)
w,h,rows=read_rgb_png(path);assert(w,h)==(240,160)
stripe=lambda y:bytes(rows[y][48*3:192*3])
colors={tuple(rows[80][x*3:x*3+3])for x in range(48,192)}
assert 2<=len(colors)<=8 and stripe(80)==stripe(88),f'{system}: synthetic cartridge stripe program did not render: {path}'
print(f'MiGoomba generic {system}: generated cartridge executes and renders its stripe pattern in stock mGBA (no BIOS/card).')
PY
done
