#!/usr/bin/env bash
# Separate deterministic boots avoid screenshots racing input/repaint.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
if (( $# == 0 )); then set -- {0..8}; fi
python3 "$root/scripts/build-native-menu-preview.py"
for scene in "$@"; do
 python3 - "$root/build" "$scene" <<'PY'
from pathlib import Path
import sys
scene=int(sys.argv[2]);assert 0<=scene<9
(Path(sys.argv[1])/"native-preview.sav").write_bytes(bytes([0xd6,scene,0xaa])+bytes(32765))
PY
 SDL_AUDIODRIVER=dummy xvfb-run -a bash -c '
  set -euo pipefail
  cd "$1"
  "$2" -1 native-preview.gba >/dev/null 2>&1 &
  preview_pid=$!
  trap "kill $preview_pid 2>/dev/null || true; wait $preview_pid 2>/dev/null || true" EXIT
  sleep 3
  xdotool key F12
  sleep 0.5
  python3 - "$3" <<PY
from pathlib import Path
import sys,struct
p=max(Path(".").glob("native-preview-*.png"),key=lambda p:p.stat().st_mtime_ns)
data=p.read_bytes();assert data[:8]==b"\x89PNG\r\n\x1a\n" and struct.unpack(">II",data[16:24])==(240,160)
assert len(data)>1000,"Unexpected blank preview capture"
Path("native-scene-"+sys.argv[1]+".png").write_bytes(data)
PY
 ' _ "$root/build" "${MGBA_BIN:-/usr/games/mgba}" "$scene"
 python3 - "$root/build" "$scene" <<'PY'
from pathlib import Path
import sys,json,hashlib
state=(Path(sys.argv[1])/'native-preview.sav').read_bytes()
assert state[:3]==bytes([0xd5,int(sys.argv[2]),0xaa]),'Native renderer did not finish requested scene'
out=Path(sys.argv[1]);scene=sys.argv[2]
record=json.loads((out/'native-preview.json').read_text())
record['scene']=int(scene)
record['screenshot_sha256']=hashlib.sha256((out/('native-scene-'+scene+'.png')).read_bytes()).hexdigest()
(out/('native-scene-'+scene+'.json')).write_text(json.dumps(record,indent=2)+'\n')
PY
done
printf 'Requested native renderer scenes captured; visual fixtures only, no card qualification.\n'
