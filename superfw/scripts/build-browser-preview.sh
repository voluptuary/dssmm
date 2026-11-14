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
python3 - "$project_root/src/menu.c" "$out/browser_preview_text.h" <<'PY'
from pathlib import Path
import sys
source=Path(sys.argv[1]).read_text()
start=source.index("static void draw_text_ovf_rotate(")
end=source.index("\n}",start)+2
Path(sys.argv[2]).write_text("#define SCREEN_WIDTH 240\n#define FT_COLOR 1\n#define ANIM_INITIAL_WAIT 128\n"+source[start:end]+"\n")
PY
arm-none-eabi-gcc -O2 -Wall -Wextra -Werror -Wno-old-style-declaration -mcpu=arm7tdmi -mthumb \
  -D__GBA__ -DPREVIEW_VERTICAL="${PREVIEW_VERTICAL:-0}" -DPREVIEW_BOX="${PREVIEW_BOX:-0}" -DPREVIEW_OFFSET="${PREVIEW_OFFSET:-0}" -DPREVIEW_AUTOMOVE="${PREVIEW_AUTOMOVE:-0}" -I "$project_root/src" -I "$out" -specs=gba.specs \
  "$project_root/tests/browser-preview-rom.c" "$project_root/src/browser_carousel.c" \
  "$project_root/src/artwork.c" "$project_root/src/gbahw.c" \
  "$project_root/src/fonts/font_render.c" "$project_root/src/utf_util.c" \
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
