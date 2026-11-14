#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
out="$project_root/build/mgba"
mkdir -p "$out"
if [ -n "${DEVKITARM:-}" ] && [ -x "$DEVKITARM/bin/arm-none-eabi-gcc" ]; then
  export PATH="$DEVKITARM/bin:$PATH"
fi
python3 "$project_root/scripts/generate-browser-fixtures.py" \
  "$out/fixtures" --header "$out/browser_fixture_assets.h"
arm-none-eabi-gcc -O2 -Wall -Wextra -Werror -mcpu=arm7tdmi -mthumb \
  -D__GBA__ -DPREVIEW_VERTICAL="${PREVIEW_VERTICAL:-0}" -DPREVIEW_BOX="${PREVIEW_BOX:-0}" -I "$project_root/src" -I "$out" -specs=gba.specs \
  "$project_root/tests/browser-preview-rom.c" "$project_root/src/browser_carousel.c" \
  "$project_root/src/artwork.c" "$project_root/src/gbahw.c" \
  -o "$out/browser-preview.elf"
arm-none-eabi-objcopy -O binary "$out/browser-preview.elf" "$out/browser-preview.gba"
python3 - "$out/browser-preview.gba" <<'PY'
from pathlib import Path
import sys
path = Path(sys.argv[1])
rom = bytearray(path.read_bytes())
rom[0xBD] = (-0x19 - sum(rom[0xA0:0xBD])) & 0xFF
path.write_bytes(rom)
PY
printf 'Built mGBA browser preview: %s\n' "$out/browser-preview.gba"
