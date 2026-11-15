#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
out="$project_root/build/mgba"
mkdir -p "$out"
python3 "$project_root/scripts/generate-ingame-preview.py" "$out/ingame_preview.h"
python3 "$project_root/res/messages.py" h main > "$out/messages_data.h"
cp "$project_root/src/messages.h" "$out/messages.h"
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
for signature in ("static void draw_text_ovf(", "static void draw_central_text("):
 start=source.index(signature);end=source.index("\n}",start)+2
 text+=source[start:end]+"\n"
text+='#include "messages.h"\nstatic unsigned lang_id;\nstatic struct { unsigned anim_state; struct {unsigned selector;} tools; struct {unsigned selector;} set; struct {int selector,seloff;} uiset; } smenu;\n#define ToolsMAX 5\nstatic void render_icon_trans(unsigned x,unsigned y,unsigned icon) {(void)x;(void)y;(void)icon;}\n'
text+="extern void dma_memset16(volatile void *, uint16_t, uint16_t);\n#define dup8(x) ((x)|((x)<<8))\n#define npf_snprintf snprintf\n#define THEME_COUNT 8\n#define ACCENT_COUNT 5\n#define THEME_DSSMM_LIGHT 6\n#define THEME_DSSMM_DARK 7\n"
text+="static uint8_t menu_theme=6,artwork_mode=1,browser_view=1,recent_menu=1,anim_speed=1,hide_hidden=1,menu_accent;\n"
text+='static const char * const accent_names[5]={"Default","Blue","Green","Orange","Purple"};\n'
a=source.index("enum {\n  UiSetTheme");b=source.index("\n};",a)+3
text+=source[a:b]+"\n"
for signature in ("static void draw_box_outline(","static void draw_box_full(","static void draw_button_box(","static void draw_menu_selection(","void render_ui_settings("):
 start=source.index(signature);end=source.index("\n}",start)+2
 text+=source[start:end]+"\n"
a=source.index("  if (smenu.uiset.selector == UiSetSave)",source.index("static void keypress_menu_uisettings("))
b=source.index("  // Apply settings immediately",a)
text+="static void preview_select_ui(unsigned option) {\nsmenu.uiset.selector=option>UiSetSave?UiSetSave:option;\n"+source[a:b]+"}\n"
settings=Path(sys.argv[1]).with_name("settings.c").read_text()
header=Path(sys.argv[1]).with_name("settings.h").read_text()
a=header.index("typedef struct {\n  uint16_t fg_color");b=header.index("} t_theme_colors;",a)+len("} t_theme_colors;")
text+=header[a:b]+"\n#define THEME_DSSMM_LIGHT 6\n#define THEME_DSSMM_DARK 7\n"
text+="#define RGB2GBA(c) ((((c)&0xF80000)>>19)|(((c)&0x00F800)>>6)|(((c)&0x0000F8)<<7))\n"
a=settings.index("const t_theme_colors themes[");b=settings.index("\n};",a)+3
text+=settings[a:b]+"\n"
start=source.index("void render_tools(");end=source.index("\n}",start)+2
text+=source[start:end]+"\n"
a=source.index("enum {\n  SettTitle1");b=source.index("\n};",a)+3
text+=source[a:b]+"\n"
text+='#include "settings-preview-fixture.h"\n'
a=source.index("void render_settings(");b=source.index("\n}",a)+2
text+=source[a:b]+"\n"
text+="static struct {unsigned anim; const char *alert_msg;} spop;\nstatic const char *file_basename(const char *path) {const char *p=strrchr(path,'/');return p?p+1:path;}\n"
a=source.index("static void render_gbarom_info(");b=source.index("\n}",a)+2
text+=source[a:b]+"\n"
a=source.index("enum {\n  GBAInfoCNT");b=source.index("\n};",a)+3
text+=source[a:b]+"\n"
for signature in ("static const char *render_gbarom_patching(","static const char *render_gbarom_loading("):
 a=source.index(signature);b=source.index("\n}",a)+2
 text+=source[a:b]+"\n"
text+="\n#define KEY_BUTTA 1\n#define KEY_BUTTUP 64\n#define KEY_BUTTDOWN 128\n#define KEY_BUTTLEFT 32\n#define KEY_BUTTRIGHT 16\n#define animspd_cnt 5\n#define MAX(a,b) ((a)>(b)?(a):(b))\n#define MIN(a,b) ((a)<(b)?(a):(b))\n"
text+="static int browser_motion_offset;\nstatic void browser_invalidate_artwork(void) {}\nstatic void browser_reload_filter(void) {}\nstatic bool save_ui_settings(void) {return false;} /* No SD persistence in fixture. */\nstatic void reload_theme(unsigned theme);\n"
a=source.index("static void keypress_menu_uisettings(");b=source.index("\n}",a)+2
text+=source[a:b]+"\n"
a=settings.index("static const uint16_t accent_colors[");b=settings.index("\n};",a)+3
text+=settings[a:b]+"\n"
a=settings.index("t_theme_colors menu_theme_colors(");b=settings.index("\n}",a)+2
text+=settings[a:b]+"\n"
Path(sys.argv[2]).write_text(text)
PY
arm-none-eabi-gcc -O2 -Wall -Wextra -Werror -Wno-old-style-declaration -Wno-unused-parameter -mcpu=arm7tdmi -mthumb \
  -D__GBA__ -DPREVIEW_MAKEPERS="${PREVIEW_MAKEPERS:--1}" -DPREVIEW_STATE_SLOT="${PREVIEW_STATE_SLOT:-0}" ${PREVIEW_CHIS:+-DSUPPORT_NORGAMES} -I "$project_root/tests" -DPREVIEW_THEME="${PREVIEW_THEME:--1}" -DPREVIEW_OPTION="${PREVIEW_OPTION:-0}" -DPREVIEW_TIMING="${PREVIEW_TIMING:-0}" -DPREVIEW_SCREEN="${PREVIEW_SCREEN:-0}" -DPREVIEW_VERTICAL="${PREVIEW_VERTICAL:-0}" -DPREVIEW_BOX="${PREVIEW_BOX:-0}" -DPREVIEW_OFFSET="${PREVIEW_OFFSET:-0}" -DPREVIEW_AUTOMOVE="${PREVIEW_AUTOMOVE:-0}" -I "$project_root/src" -I "$out" -specs=gba.specs \
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
