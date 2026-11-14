#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
out="$project_root/build/mgba"
mkdir -p "$out"
python3 "$project_root/res/messages.py" h main > "$out/messages_data.h"
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
text="#define SCREEN_WIDTH 240\n#define FT_COLOR 1\n#define FG_COLOR 3\n#define HI_COLOR 2\n#define ANIM_INITIAL_WAIT 128\n#define THREEDOTS_WIDTH 9\n"+source[start:end]+"\n"
for signature in ("static void draw_text_ovf(", "static void draw_central_text(", "static void render_artwork_placeholder("):
 start=source.index(signature);end=source.index("\n}",start)+2
 text+=source[start:end]+"\n"
text+='#include "messages.h"\nstatic unsigned lang_id;\nstatic struct { unsigned anim_state; struct {unsigned selector;} tools; struct {unsigned selector,seloff;} uiset; } smenu;\n#define ToolsMAX 5\nstatic void render_icon_trans(unsigned x,unsigned y,unsigned icon) {(void)x;(void)y;(void)icon;}\n'
start=source.index("void render_tools(");end=source.index("\n}",start)+2
text+=source[start:end]+"\n"
text+="extern void dma_memset16(volatile void *, uint16_t, uint16_t);\n#define dup8(x) ((x)|((x)<<8))\n#define npf_snprintf snprintf\n#define THEME_COUNT 8\n#define ACCENT_COUNT 5\n"
text+="static uint8_t menu_theme=6,artwork_mode=1,browser_view=1,carousel_animation=0,recent_menu=1,anim_speed=1,hide_hidden=1,menu_accent;\n"
text+='static const char * const accent_names[5]={"Default","Blue","Green","Orange","Purple"};\n'
a=source.index("enum {\n  UiSetTheme");b=source.index("\n};",a)+3
text+=source[a:b]+"\n"
for signature in ("static void draw_box_outline(","static void draw_box_full(","static void draw_button_box(","void render_ui_settings("):
 start=source.index(signature);end=source.index("\n}",start)+2
 text+=source[start:end]+"\n"
Path(sys.argv[2]).write_text(text)
PY
arm-none-eabi-gcc -O2 -Wall -Wextra -Werror -Wno-old-style-declaration -mcpu=arm7tdmi -mthumb \
  -D__GBA__ -DPREVIEW_ANIMATION="${PREVIEW_ANIMATION:-1}" -DPREVIEW_TIMING="${PREVIEW_TIMING:-0}" -DPREVIEW_SCREEN="${PREVIEW_SCREEN:-0}" -DPREVIEW_VERTICAL="${PREVIEW_VERTICAL:-0}" -DPREVIEW_BOX="${PREVIEW_BOX:-0}" -DPREVIEW_OFFSET="${PREVIEW_OFFSET:-0}" -DPREVIEW_AUTOMOVE="${PREVIEW_AUTOMOVE:-0}" -I "$project_root/src" -I "$out" -specs=gba.specs \
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
