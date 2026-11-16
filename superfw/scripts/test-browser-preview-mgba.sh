#!/usr/bin/env bash
set -euo pipefail
if [[ -v PREVIEW_ANIMATION || -v PREVIEW_EXPECT_ANIMATION ]]; then
  echo "The Animation toggle/instant mode was retired; omit PREVIEW_ANIMATION and PREVIEW_EXPECT_ANIMATION." >&2
  exit 2
fi
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
python3 - "$out" "$project_root/tests" <<'PY'
import os
from collections import Counter
from pathlib import Path
import sys

out = Path(sys.argv[1])
screenshots = sorted(out.glob("browser-preview*.png"), key=lambda p: p.stat().st_mtime)
if not screenshots:
    raise SystemExit("mGBA did not save a browser-preview screenshot")
path = screenshots[-1]
sys.path.insert(0,sys.argv[2])
from png_reader import read_rgb_png,artwork_pixel_count
width,height,rows=read_rgb_png(path)
if (width,height)!=(240,160):raise SystemExit("Expected240x160 mGBA screenshot")
stride=width*3
pixels = Counter(tuple(row[i:i + 3]) for row in rows for i in range(0, stride, 3))
# Actual BMP quantization uses the six-level RGB cube. Neutral DS backgrounds
# must never count as artwork merely because they contain many gray shades.
art_pixels=artwork_pixel_count(rows)
if os.environ.get("PREVIEW_SCREEN") in ("1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14"):
    background=tuple(rows[80][-3:])
    text_pixels = sum(tuple(row[i:i+3]) != background for row in rows[20:140] for i in range(0,stride,3))
    if text_pixels < 200: raise SystemExit("Menu text missing")
    print(f"Production menu text rendered: {text_pixels} body text pixels")
    sys.exit(0)
if len(pixels) < 8 or art_pixels < 500:
    raise SystemExit(f"Preview lacks decoded BMP pixels: {len(pixels)} colors, {art_pixels} artwork pixels")
print(f"mGBA preview rendered {width}x{height}; {len(pixels)} colors; chromatic synthetic-art pixels: {art_pixels}")
PY
