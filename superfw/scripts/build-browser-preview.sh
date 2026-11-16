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
text="#define MAX(a,b) ((a)>(b)?(a):(b))\n#define KEY_BUTTUP 64\n#define KEY_BUTTDOWN 128\n#define MIN(a,b) ((a)<(b)?(a):(b))\n#define SCREEN_WIDTH 240\n#define FT_COLOR 18\n#define FG_COLOR 16\n#define HI_COLOR 19\n#define SELECTED_TEXT 236\n#define MEM_PALETTE ((volatile uint16_t *)0x05000000)\n#define ANIM_INITIAL_WAIT 128\n#define THREEDOTS_WIDTH 9\n"+source[start:end]+"\n"
for signature in ("static void draw_text_ovf_color(", "static void draw_text_ovf(", "static void draw_central_text(", "static void draw_central_text_ovf(", "static void draw_rightj_text("):
 start=source.index(signature);end=source.index("\n}",start)+2
 text+=source[start:end]+"\n"
text+='#include "messages.h"\nstatic unsigned lang_id;\nstatic struct { unsigned anim_state; struct {unsigned selector;} tools; struct {unsigned selector;} set; unsigned menu_tab; struct {int selector,seloff,maxentries;} recent; struct {int selector,seloff;} uiset; } smenu;\n#define ToolsMAX 5\nstatic void render_icon_trans(unsigned x,unsigned y,unsigned icon) {(void)x;(void)y;(void)icon;}\n'
text+="extern void dma_memset16(volatile void *, uint16_t, uint16_t);\n#define dup8(x) ((x)|((x)<<8))\n#define npf_snprintf snprintf\n#define THEME_COUNT 8\n#define ACCENT_COUNT 5\n#define THEME_DSSMM_LIGHT 6\n#define THEME_DSSMM_DARK 7\n"
text+="static uint8_t menu_theme=6,artwork_mode=1,browser_view=1,recent_menu=1,anim_speed=1,hide_hidden=1,menu_accent,menu_font,clean_list;\n"
text+='static void render_dssmm_settings_background(volatile uint8_t *,const char *);\n'
for signature in ('static unsigned settings_visible_rows(', 'static unsigned ui_visible_rows('):
 a=source.index(signature);b=source.index('\n}',a)+2
 text+=source[a:b]+'\n'
text+='static const char * const accent_names[5]={"Default","Blue","Green","Orange","Purple"};\n'
a=source.index("enum {\n  UiSetTheme");b=source.index("\n};",a)+3
text+=source[a:b]+"\n"
for signature in ("static void draw_box_outline(","static void draw_box_full(","static void draw_menu_selection(","void render_ui_settings("):
 start=source.index(signature);end=source.index("\n}",start)+2
 text+=source[start:end]+"\n"
a=source.index("  if (smenu.uiset.selector < smenu.uiset.seloff)",source.index("static void keypress_menu_uisettings("))
b=source.index("  // Apply settings immediately",a)
text+="static void preview_select_ui(unsigned option) {\nunsigned rows=ui_visible_rows();\nsmenu.uiset.selector=option>UiSetMAX?UiSetMAX:option;\n"+source[a:b]+"}\n"
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
text+='static uint8_t launcher_settings_group;\n'
a=source.index('/* Filter settings by category');b=source.index('void render_settings(',a)
text+=source[a:b]+"\n"
text+='#include "settings-preview-fixture.h"\n'
a=source.index("void render_settings(");b=source.index("\n}",a)+2
text+=source[a:b]+"\n"
text+="static struct {unsigned anim; const char *alert_msg;struct {t_dec_date val;int selector;bool menu_clock;void (*callback)(void);} rtcpop;} spop;\nstatic const char *file_basename(const char *path) {const char *p=strrchr(path,'/');return p?p+1:path;}\n"
a=source.index("static void render_gbarom_info(");b=source.index("\n}",a)+2
text+=source[a:b]+"\n"
a=source.index("enum {\n  GBAInfoCNT");b=source.index("\n};",a)+3
text+=source[a:b]+"\n"
for signature in ("static const char *render_gbarom_patching(","static const char *render_gbarom_loading("):
 a=source.index(signature)
 while ';' in source[a:source.index('{',a)]:a=source.index(signature,a+len(signature))
 b=source.index("\n}",a)+2
 text+=source[a:b]+"\n"
text+="\n#define KEY_BUTTA 1\n#define KEY_BUTTUP 64\n#define KEY_BUTTDOWN 128\n#define KEY_BUTTLEFT 32\n#define KEY_BUTTRIGHT 16\n#define animspd_cnt 5\n#define MAX(a,b) ((a)>(b)?(a):(b))\n#define MIN(a,b) ((a)<(b)?(a):(b))\n"
text+="static int browser_motion_offset;\nstatic void browser_invalidate_artwork(void) {}\nstatic void browser_reload_filter(void) {}\nstatic void reload_theme(unsigned theme);\n"
a=source.index("static void keypress_menu_uisettings(");b=source.index("\n}",a)+2
text+=source[a:b]+"\n"
a=settings.index("static const uint16_t accent_colors[");b=settings.index("\n};",a)+3
text+=settings[a:b]+"\n"
a=settings.index("t_theme_colors menu_theme_colors(");b=settings.index("\n}",a)+2
text+=settings[a:b]+"\n"
text+='#define strbcpy(d,s,n) snprintf(d,n,"%s",s)\n#include "dssmm_background.h"\n#include "launcher_profile.h"\n#include "recent.h"\n'
utilities=Path(sys.argv[1]).with_name("util.c").read_text()
text+=utilities[utilities.index("void clean_display_name("):]+"\n"
text+='static uint8_t launcher_page=1,launcher_selected=1,launcher_category,launcher_browser_panel=1;\nstatic void launcher_invalidate(void) {}\nstatic char menu_username[LAUNCHER_NAME_BYTES]="Test User";\n'
a=source.index("enum {\n  MENUTAB_RECENT");b=source.index("\n};",a)+3
text+=source[a:b]+"\n"
text+='#define KEY_BUTTB 2\n#define KEY_BUTTSTA 8\n'
text+='static struct {t_rentry rentries[1];uint16_t artwork_pixels[ARTWORK_MAX_PIXELS/2];uint8_t artwork_bmp_buffer[ARTWORK_BMP_MAX_SIZE];} preview_sdram __attribute__((section(".ewram"),aligned(4)));\n#define sdr_state (&preview_sdram)\nstatic bool artwork_cached,artwork_valid;\nstatic unsigned artwork_cached_mode;\nstatic char artwork_rom[256];\nstatic artwork_info preview_info;\n'
text+='static bool preview_load_art(const char *,unsigned,uint8_t *,size_t,uint8_t *,size_t,artwork_info *);\n#define artwork_load preview_load_art\n'
for signature in ("static void launcher_corner_span(","static void launcher_corners(","static void render_start(","static void render_start_bar(","static void render_dssmm_settings_header(","static void render_dssmm_settings_background(","static void render_settings_home(","void render_rtcpop("):
 a=source.index(signature)
 while ';' in source[a:source.index('{',a)]:a=source.index(signature,a+len(signature))
 b=source.index("\n}",a)+2
 text+=source[a:b]+"\n"
text+='static void accept_menu_clock(void) {launcher_clock_set((unsigned)spop.rtcpop.val.hour*3600+(unsigned)spop.rtcpop.val.min*60);}\n'
for signature in ('static void open_menu_clock(', 'static void launcher_move('):
 a=source.index(signature);b=source.index('\n}',a)+2
 text+=source[a:b]+'\n'
text+='static void fixdate(t_dec_date *v) {if(v->hour<0)v->hour=23;if(v->hour>23)v->hour=0;if(v->min<0)v->min=59;if(v->min>59)v->min=0;}\n'
a=source.index('    if (newkeys & KEY_BUTTLEFT)',source.index('  else if (spop.rtcpop.callback)',source.index('void menu_keypress(')))
b=source.index('  }\n  else if (spop.pop_num)',a)
text+='static void preview_clock_keypress(unsigned newkeys) {\n'+source[a:b]+'}\n'
text+='static unsigned preview_recent_actions;\nstatic void keypress_menu_recent(unsigned keys) {if(keys&KEY_BUTTA)++preview_recent_actions;}\n'
a=source.index('    if(menu_theme>=THEME_DSSMM_LIGHT) {',source.index('void menu_keypress('))
b=source.index('    // Menu change via trigger buttons',a)
text+='static void preview_launcher_keypress(unsigned newkeys) {\n'+source[a:b]+'}\n'
Path(sys.argv[2]).write_text(text)
PY
arm-none-eabi-gcc -O2 -Wall -Wextra -Werror -Wno-old-style-declaration -Wno-unused-parameter -mcpu=arm7tdmi -mthumb \
  -D__GBA__ -DPREVIEW_FONT="${PREVIEW_FONT:-0}" -DPREVIEW_MAKEPERS="${PREVIEW_MAKEPERS:--1}" -DPREVIEW_STATE_SLOT="${PREVIEW_STATE_SLOT:-0}" ${PREVIEW_CHIS:+-DSUPPORT_NORGAMES} -I "$project_root/tests" -DPREVIEW_THEME="${PREVIEW_THEME:--1}" -DPREVIEW_OPTION="${PREVIEW_OPTION:-0}" -DPREVIEW_TIMING="${PREVIEW_TIMING:-0}" -DPREVIEW_SCREEN="${PREVIEW_SCREEN:-0}" -DPREVIEW_VERTICAL="${PREVIEW_VERTICAL:-0}" -DPREVIEW_BOX="${PREVIEW_BOX:-0}" -DPREVIEW_OFFSET="${PREVIEW_OFFSET:-0}" -DPREVIEW_AUTOMOVE="${PREVIEW_AUTOMOVE:-0}" -I "$project_root/src" -I "$out" -specs=gba.specs \
  "$project_root/tests/browser-preview-rom.c" "$project_root/src/browser_carousel.c" \
  "$project_root/src/artwork.c" "$project_root/src/launcher_profile.c" "$project_root/src/dssmm_background.c" "$project_root/src/gbahw.c" \
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
