#!/usr/bin/env bash
# Actual supplied portable plugins and synthetic ROMs; no SuperCard emulation.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
pack="${1:-$root/emu}"
for system in ${DSSMM_PLUGIN_SYSTEMS:-sms gg pce pce-native}; do
 kind="${system%-native}"
 options=(--system "$kind")
 plugin="$pack/sms.mb"
 [[ "$kind" == pce ]] && plugin="$pack/pce.mb"
 if [[ ! -f "$plugin" ]]; then
  plugin="$root/emu/smsadvance-v2.5-scptch.gba.bin"
  [[ "$kind" == pce ]] && plugin="$root/emu/pceadvance-v7.5-scptch.gba.bin"
 fi
 if [[ "$system" == pce-native ]]; then plugin="$root/emu/pceadvance-v7.5-scptch.gba.bin"; options+=(--native-pce); fi
 out="$root/build/plugin-preview-$system"
 python3 "$root/scripts/build-plugin-preview.py" "$plugin" "${options[@]}" --out "$out"
 # Reset only this generated fixture's SRAM so menu selection remains deterministic.
 python3 - "$out" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1])/'preview.sav'
if p.exists():p.unlink()
PY
 SDL_AUDIODRIVER=dummy xvfb-run -a bash -c '
  cd "$1"
  "$2" -1 preview.gba >/dev/null 2>&1 &
  preview_pid=$!
  trap "kill $preview_pid 2>/dev/null || true; wait $preview_pid 2>/dev/null || true" EXIT
  sleep 4
  xdotool keydown a; xdotool keydown s; sleep 0.2; xdotool keyup a; xdotool keyup s
  sleep 0.3
  for step in 1 2 3 4 5 6 7 8 9; do xdotool keydown Down; sleep 0.08; xdotool keyup Down; sleep 0.08; done
  xdotool keydown x; sleep 0.2; xdotool keyup x
  sleep 1
  xdotool key F12
  sleep 1
 ' _ "$out" "${MGBA_BIN:-/usr/games/mgba}"
 python3 - "$root" "$out" "$system" <<'PY'
from pathlib import Path
import sys
root,out=map(Path,sys.argv[1:3]);sys.path.insert(0,str(root/'tests'))
from png_reader import read_rgb_png
p=max(out.glob('preview-*.png'),key=lambda p:p.stat().st_mtime)
w,h,rows=read_rgb_png(p);assert(w,h)==(240,160)
assert all(tuple(row[x*3:x*3+3])==(0,255,0) for row in rows for x in range(240)),f'{sys.argv[3]} Exit failed: {p}'
print(f'Actual {sys.argv[3]} plugin menu Exit reaches return target in stock mGBA.')
PY
done
