#!/usr/bin/env bash
# Optional source-prototype check; deliberately outside the firmware verifier.
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
out="$(realpath "${1:-$root/build/atari2600-prototype}")"
started="$(date +%s)"
SDL_AUDIODRIVER=dummy xvfb-run -a bash -c '
  set -euo pipefail
  cd "$1"
  "$2" -1 prototype.gba >mgba.log 2>&1 &
  emulator_pid=$!
  trap "kill $emulator_pid 2>/dev/null || true; wait $emulator_pid 2>/dev/null || true" EXIT
  sleep 6
  xdotool key F12
  sleep 1
' _ "$out" "${MGBA_BIN:-/usr/games/mgba}"
python3 - "$root" "$out" "$started" <<'PY'
import hashlib, json, sys
from pathlib import Path
root, out = map(Path, sys.argv[1:3])
sys.path.insert(0, str(root / 'tests'))
from png_reader import read_rgb_png
image = max(out.glob('prototype-*.png'), key=lambda p: p.stat().st_mtime_ns)
assert image.stat().st_mtime >= int(sys.argv[3]), 'No fresh capture'
w, h, rows = read_rgb_png(image)
assert (w, h) == (240, 160)
for y, row in enumerate(rows):
    for x in range(w):
        expected = (255, 198, 206) if 35 <= x < 195 and 25 <= y < 121 else (0, 0, 0)
        assert tuple(row[3*x:3*x+3]) == expected, (x, y)
record = json.loads((out / 'prototype.json').read_text())
record.update(screenshot_sha256=hashlib.sha256(image.read_bytes()).hexdigest(),
              test='stock SDL mGBA0.10.1/Xvfb, no BIOS; exact 160x96 diagnostic rectangle')
(out / 'validation.json').write_text(json.dumps(record, indent=2) + '\n')
print('Atari source prototype: generated 6502 diagnostic renders the exact expected frame.')
PY
