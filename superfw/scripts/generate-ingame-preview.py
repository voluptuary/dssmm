#!/usr/bin/env python3
"""Extract presentation-only in-game menu functions, with explicit fake state."""
from pathlib import Path
import re
import subprocess
import sys
root=Path(__file__).resolve().parents[1]
source=(root/'src/ingame_menu.c').read_text()
messages=subprocess.check_output([sys.executable,str(root/'res/messages.py'),'h','menu'],text=True,cwd=root)
# Keep main-menu and in-game translation tables in separate namespaces.
messages=re.sub(r'\b(msgs|msg_[a-z]+|lang_codes|TranslationID)\b',lambda m:'igm_'+m[0],messages)
text=messages+'''
/* Synthetic feature flags; no game, SD, save or register-restoration actions. */
static unsigned copt,selbarpos,num_mem_savestates=2,num_dsk_savestates=2;
static unsigned has_rtc_support=1,cheat_base_addr=1,ingame_menu_lang;
static t_dec_date rtc_date={26,11,15,12,0};
static unsigned rtc_speed=1;
static const char savefile_pattern[]="preview.sav";
static struct {const char *msg;} igm_popup;
#define NOSELBAR 240
'''
for name in ('draw_text','draw_text_center','draw_menu_selection','draw_main_menu','draw_reset_menu','draw_save_menu','draw_rtc_menu'):
 marker=('static void ' if name=='draw_menu_selection' else 'void ')+name+'('
 a=source.index(marker);b=source.index('\n}',a)+2
 body=source[a:b]
 replacements={'draw_text':'igm_draw_text','draw_text_center':'igm_text_center','draw_rtc_menu':'igm_draw_rtc_menu','draw_menu_selection':'igm_selection',
 'draw_main_menu':'igm_draw_main_menu','draw_reset_menu':'igm_draw_reset_menu',
 'draw_save_menu':'igm_draw_save_menu','popup':'igm_popup','msgs':'igm_msgs',
 'HI_COLOR':'FT_COLOR','BL_COLOR':'HI_COLOR','SH_COLOR':'FG_COLOR'}
 body=re.sub(r'\b('+'|'.join(replacements)+r')\b',lambda m:replacements[m[0]],body)
 text+=body+'\n'
Path(sys.argv[1]).write_text(text)
