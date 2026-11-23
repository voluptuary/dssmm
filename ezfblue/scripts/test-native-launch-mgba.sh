#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$root/build/native-launch"
mkdir -p "$out"
for case in fat-sfn exfat exfat-long quick-clean quick-addon; do
 flags=()
 case "$case" in
  exfat)flags=(--no-alias);;
  exfat-long)flags=(--no-alias --long-name);;
  quick-clean)flags=(--no-alias --mode 1);;
  quick-addon)flags=(--no-alias --mode 2);;
 esac
 python3 "$root/scripts/build-native-launch-preview.py" "${flags[@]}" > "$root/build/native-launch-$case-build.log" 2>&1
 python3 - "$out" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1])/'native-launch.sav'
if p.exists():p.unlink()
PY
 SDL_AUDIODRIVER=dummy xvfb-run -a bash -c '
  set -euo pipefail
  cd "$1"
  "${MGBA_BIN:-/usr/games/mgba}" -1 native-launch.gba >/dev/null 2>&1 &
  launch_pid=$!
  trap "kill $launch_pid 2>/dev/null || true; wait $launch_pid 2>/dev/null || true" EXIT
  sleep 3
  xdotool keydown x
  sleep 0.15
  xdotool keyup x
  sleep 2
  xdotool key F12
  sleep 1
 ' _ "$out"
 python3 - "$out" "$case" <<'PY'
from pathlib import Path
import sys,json,hashlib
out=Path(sys.argv[1]);case=sys.argv[2];saved=(out/'native-launch.sav').read_bytes()
expected=191 if case=='quick-addon' else 255
assert saved[:3]==bytes([0xd7,expected,0xaa]),f'{case}: launch stage trace {saved[:8].hex()}'
record=json.loads((out/'fixture.json').read_text());record.update(status='all launch stages reached handoff',trace=saved[:8].hex(),emulator='stock SDL mGBA0.10.1, Xvfb, no BIOS')
(out/(case+'.json')).write_text(json.dumps(record,indent=2)+'\n')
print(f'BLUE native mGBA {case}: A/quick launch resolves file and reaches all stages through page0x400 handoff (hardware seams mocked).')
PY
done
