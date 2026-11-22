#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$root/src/menu.c" "$tmp/test.c" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text();a=s.index('static void browser_reload(void);');b=s.index('\nNOINLINE static void browser_open(',a)
declarations=s[s.index('enum progress_action {'):s.index('static void render_progress_panel(')]
Path(sys.argv[2]).write_text('''#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "emu.h"
#include "util.h"
#define MAX_FN_LEN 256
#define FLAG_RECENT_SD 0
#define ERR_LOAD_NOEMU 1
#define ERR_SAVE_BADSAVE 2
#define MSG_ERR_SAVERD 0
#define MSG_ERR_SAVEWR 1
#define MSG_ERR_NOEMU 2
#define MSG_ERR_READ 3
#define SaveLoadSav 1
#define SaveLoadReset 0
#define SaveReboot 0
const t_emu_loader pogo_plugin_loader={"pogo:",0};
static const char *msgs[1][4]={{"save read","save write","no emulator","read"}};
static unsigned lang_id,loads,reloads,records,save_error,plugin_error,native_error;
static bool recent_ok=true;
typedef unsigned t_sram_load_policy;
static struct {struct {char savefn[256];} l;} load;
static struct {const char *alert_msg;struct {typeof(load) load;} p;} spop;
static struct {struct {unsigned filecount,dispentries;int selector,seloff;} browser;} smenu;
static bool insert_recent_flush(const char *fn,unsigned flags){assert(!strcmp(fn,"/Games/Dr. Mario (USA).gg")&&!flags);records++;return recent_ok;}
static void sram_filename_calc(const char *rom,char *save){(void)rom;strcpy(save,"/SYSTEM/SAVER/Dr. Mario (USA).sav");}
bool check_file_exists(const char *fn){(void)fn;return false;}
static unsigned prepare_sram_based_savegame(unsigned a,unsigned b,const char *c){(void)a;(void)b;(void)c;return save_error;}
static void loadrom_progress(unsigned a,unsigned b){(void)a;(void)b;}
static void loadpogo_progress(unsigned a,unsigned b){(void)a;(void)b;}
static const char *plugin_load_error(void){return msgs[0][MSG_ERR_READ];}
#define write_log(...) ((void)0)
static unsigned load_extemu_rom(const char *fn,unsigned fs,const t_emu_loader *ld,void(*cb)(unsigned,unsigned)){
 assert(!strcmp(fn,"/Games/Dr. Mario (USA).gg")&&fs==1234&&cb==(ld==&pogo_plugin_loader?loadpogo_progress:loadrom_progress));loads++;
 return ld==&pogo_plugin_loader?plugin_error:native_error;
}
static void browser_invalidate_artwork(void){}
static void browser_reload(void){assert(!smenu.browser.filecount&&!smenu.browser.dispentries&&!smenu.browser.selector&&!smenu.browser.seloff);reloads++;}
static void flashbrowser_reload(void){reloads++;}
static void recent_reload(void){reloads++;}
'''+declarations+s[a:b]+'''
int main(void){const t_emu_loader native[]={{"sms",0},{0,0}};
 plugin_error=3;start_emu_game(native,"/Games/Dr. Mario (USA).gg",1234);
 assert(loads==1&&records==1&&reloads==3&&spop.alert_msg==msgs[0][MSG_ERR_READ]);
 loads=records=reloads=0;plugin_error=ERR_LOAD_NOEMU;native_error=3;
 start_emu_game(native,"/Games/Dr. Mario (USA).gg",1234);
 assert(loads==2&&records==1&&reloads==3&&spop.alert_msg==msgs[0][MSG_ERR_READ]);
 loads=records=reloads=0;save_error=ERR_SAVE_BADSAVE;
 start_emu_game(native,"/Games/Dr. Mario (USA).gg",1234);
 assert(!loads&&records==1&&!reloads&&spop.alert_msg==msgs[0][MSG_ERR_SAVERD]);
 loads=records=reloads=0;save_error=0;recent_ok=false;
 start_emu_game(native,"/Games/Dr. Mario (USA).gg",1234);
 assert(!loads&&records==1&&!reloads&&spop.alert_msg);
 puts("Production failed emulator loads rebuild overwritten SDRAM tables; save/recent failures stop safely.");}
''')
PY
gcc -O1 -g -fsanitize=address,undefined -fno-pie -no-pie -ffunction-sections -fdata-sections \
 -I "$root/src" "$tmp/test.c" "$root/src/util.c" -Wl,--gc-sections -o "$tmp/test"
"$tmp/test"
