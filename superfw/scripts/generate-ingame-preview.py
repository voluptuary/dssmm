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
static unsigned copt,selbarpos;
static int num_mem_savestates=2,num_dsk_savestates=2;
static unsigned has_rtc_support=1,ingame_menu_lang;
static volatile uintptr_t cheat_base_addr=1;
static t_dec_date rtc_date={26,11,15,12,0,0};
static unsigned rtc_speed=1;
static const char savefile_pattern[]="preview.sav";
static struct {const char *msg;} igm_popup;
#define NOSELBAR 240
#include "cheats.h"
#include "res/icons-menu.h"
static unsigned franim;
static int makepers=PREVIEW_MAKEPERS,state_slot=PREVIEW_STATE_SLOT;
static uint8_t memslot_valid[32]={1,0},diskslot_valid[8]={1,0};
#define DISK_ICON 1
#define DISK_ICON_DISABLED 2
#define MEM_ICON 3
#define MEM_ICON_DISABLED 4
static void igm_set16(uint16_t *dst,uint16_t value,unsigned count) {while(count--)*dst++=value;}
static void igm_copy16(uint16_t *dst,const uint16_t *src,unsigned count) {while(count--)*dst++=*src++;}
static _Alignas(4) uint8_t cheat_fixture[4+2*(4+32)];
static void igm_fake_cheats(void) {
 *(uint32_t *)cheat_fixture=2;
 for(unsigned i=0;i<2;++i) {
  t_cheathdr *entry=(t_cheathdr *)(cheat_fixture+4+i*36);
  entry->slen=8;entry->enabled=!i;
  strcpy((char *)entry->data,i ? "Synthetic cheat two" : "Synthetic cheat one");
 }
 cheat_base_addr=(uint32_t)(uintptr_t)cheat_fixture;
}
'''
for name in ('draw_hline','draw_vline','draw_text','draw_text_ovf','draw_text_ovf_rotate','draw_text_center','draw_menu_selection','draw_main_menu','draw_reset_menu','draw_save_menu','draw_rtc_menu','draw_cheats_menu','draw_icon','draw_states_menu'):
 marker=('static void ' if name in ('draw_menu_selection','draw_hline','draw_vline','draw_text_ovf','draw_text_ovf_rotate','draw_icon') else 'void ')+name+'('
 a=source.index(marker);b=source.index('\n}',a)+2
 body=source[a:b]
 replacements={'draw_text':'igm_draw_text','draw_hline':'igm_hline','draw_vline':'igm_vline','draw_icon':'igm_icon','memory_set16':'igm_set16','memory_copy16':'igm_copy16','draw_states_menu':'igm_draw_states_menu','draw_cheats_menu':'igm_draw_cheats_menu','draw_text_ovf':'igm_text_ovf','draw_text_ovf_rotate':'igm_text_rotate','draw_text_center':'igm_text_center','draw_rtc_menu':'igm_draw_rtc_menu','draw_menu_selection':'igm_selection',
 'draw_main_menu':'igm_draw_main_menu','draw_reset_menu':'igm_draw_reset_menu',
 'draw_save_menu':'igm_draw_save_menu','popup':'igm_popup','msgs':'igm_msgs',
 'HI_COLOR':'FT_COLOR','BL_COLOR':'HI_COLOR','SH_COLOR':'FG_COLOR'}
 body=re.sub(r'\b('+'|'.join(replacements)+r')\b',lambda m:replacements[m[0]],body)
 text+=body+'\n'
Path(sys.argv[1]).write_text(text)
