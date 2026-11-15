#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
out="$project_root/build/mgba"
emulator="${MGBA_BIN:-/usr/games/mgba}"
if [ "${PREVIEW_MATRIX:-0}" = 1 ]; then
  for vertical in 0 1; do
    for box in 0 1; do
      PREVIEW_MATRIX=0 PREVIEW_VERTICAL="$vertical" PREVIEW_BOX="$box" bash "$0"
      screenshot=$(find "$out" -maxdepth 1 -name 'browser-preview*.png' -print -quit)
      cp "$screenshot" "$out/view-${vertical}-${box}.png"
    done
  done
  exit 0
fi
for tool in xvfb-run xdotool python3; do
  command -v "$tool" >/dev/null || { echo "Missing preview tool: $tool" >&2; exit 1; }
done
[ -x "$emulator" ] || { echo "mGBA executable not found: $emulator" >&2; exit 1; }
"$emulator" --version
bash "$project_root/scripts/build-browser-preview.sh"
rm -f "$out"/browser-preview*.png
SDL_AUDIODRIVER=dummy xvfb-run -a bash -c '
  set -euo pipefail
  cd "$1"
  "$2" -1 browser-preview.gba >/dev/null 2>&1 &
  emulator_pid=$!
  finish() { kill "$emulator_pid" 2>/dev/null || true; wait "$emulator_pid" 2>/dev/null || true; }
  trap finish EXIT
  # First startup in a fresh Xvfb session can take several seconds.
  sleep 5
  if [ -n "${PREVIEW_KEYS:-}" ]; then
    for key in $PREVIEW_KEYS; do
      xdotool keydown --clearmodifiers "$key"
      sleep 0.1
      xdotool keyup "$key"
      sleep 0.2
    done
  fi
  xdotool key F12
  sleep 1
' _ "$out" "$emulator"
python3 - "$out" <<'PY'
import os
from collections import Counter
from pathlib import Path
import struct
import sys
import zlib

out = Path(sys.argv[1])
screenshots = sorted(out.glob("browser-preview*.png"), key=lambda p: p.stat().st_mtime)
if not screenshots:
    raise SystemExit("mGBA did not save a browser-preview screenshot")
path = screenshots[-1]
data = path.read_bytes()
if data[:8] != b"\x89PNG\r\n\x1a\n":
    raise SystemExit("mGBA screenshot is not a PNG")
offset, chunks, header = 8, [], None
while offset < len(data):
    size = struct.unpack_from(">I", data, offset)[0]
    kind = data[offset + 4:offset + 8]
    payload = data[offset + 8:offset + 8 + size]
    offset += size + 12
    if kind == b"IHDR":
        header = struct.unpack(">IIBBBBB", payload)
    elif kind == b"IDAT":
        chunks.append(payload)
    elif kind == b"IEND":
        break
if not header:
    raise SystemExit("PNG has no IHDR")
width, height, depth, color_type, compression, filtering, interlace = header
if (width, height, depth, color_type, interlace) != (240, 160, 8, 2, 0):
    raise SystemExit(f"Unexpected screenshot format: {header}; expected 240x160, 8-bit RGB, non-interlaced")
raw = zlib.decompress(b"".join(chunks))
stride, bpp = width * 3, 3
if len(raw) != height * (stride + 1):
    raise SystemExit("PNG pixel data has an unexpected length")
rows, previous = [], bytearray(stride)
pos = 0
for _ in range(height):
    method = raw[pos]
    row = bytearray(raw[pos + 1:pos + 1 + stride])
    pos += stride + 1
    for i in range(stride):
        left = row[i - bpp] if i >= bpp else 0
        above = previous[i]
        upper_left = previous[i - bpp] if i >= bpp else 0
        if method == 1:
            row[i] = (row[i] + left) & 255
        elif method == 2:
            row[i] = (row[i] + above) & 255
        elif method == 3:
            row[i] = (row[i] + ((left + above) // 2)) & 255
        elif method == 4:
            estimate = left + above - upper_left
            distances = (abs(estimate - left), abs(estimate - above), abs(estimate - upper_left))
            predictor = (left, above, upper_left)[distances.index(min(distances))]
            row[i] = (row[i] + predictor) & 255
        elif method != 0:
            raise SystemExit(f"Unsupported PNG filter {method}")
    rows.append(row)
    previous = row
if "PREVIEW_EXPECT_ANIMATION" in os.environ:
    if os.environ.get("PREVIEW_SCREEN") != "2": raise SystemExit("Animation input assertion requires UI screen2")
    expected=(255,255,255) if int(os.environ["PREVIEW_EXPECT_ANIMATION"]) else (0,0,0)
    actual=tuple(rows[159][:3])
    if actual!=expected: raise SystemExit(f"Animation state marker {actual}, expected {expected}")
    print("Production UI input animation state verified.")
pixels = Counter(tuple(row[i:i + 3]) for row in rows for i in range(0, stride, 3))
# Exclude schematic UI colors; other colors originate in decoded BMPs.
ui_colors = {(24, 57, 49), (255, 255, 255), (255, 165, 123), (173, 173, 173)}
art_pixels = sum(count for color, count in pixels.items() if color not in ui_colors)
if os.environ.get("PREVIEW_SCREEN") in ("1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"):
    background=tuple(rows[80][-3:])
    text_pixels = sum(tuple(row[i:i+3]) != background for row in rows[20:140] for i in range(0,stride,3))
    if text_pixels < 200: raise SystemExit("Menu text missing")
    print(f"Production menu text rendered: {text_pixels} body text pixels")
    sys.exit(0)
if len(pixels) < 8 or art_pixels < 500:
    raise SystemExit(f"Preview lacks decoded BMP pixels: {len(pixels)} colors, {art_pixels} artwork pixels")
print(f"mGBA preview rendered {width}x{height}; {len(pixels)} colors; decoded BMP pixels: {art_pixels}")
PY
