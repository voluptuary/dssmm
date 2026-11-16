/* Production NOR confirmation flow with flash and filesystem operations mocked. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define NOR_BLOCK_SIZE (4u*1024*1024)
#define NOR_GAMEBLOCK_COUNT 31u
#define MAX_GBA_ROM_SIZE (32u*1024*1024)
#define FLASHG_MAXFN_CNT 32u
#define ROM_FLASHMETA_ADDR 0
#define FLASH_METADATA_SIZE 0
#define FA_READ 1
#define FR_OK 0
#define MSG_ERR_NORUPD 0
#define MSG_ERR_NORSPC 1
#define MSG_Q_NO 2
#define MSG_Q_YES 3
#define MSG_ERR_READ 4
static const char *msgs[1][5]={{"write error","space error","No","Yes","read error"}};
static unsigned lang_id;
static struct { struct { struct { struct { unsigned romfs; const char *romfn; bool use_dsaving,ingame_menu_enabled; } i; } norwr; } p;
 const char *alert_msg;
 struct { const char *message,*default_button,*confirm_button; unsigned option; void (*callback)(bool); bool clear_popup_ok; } qpop;
} spop;
static struct { struct {unsigned freeblks,maxentries;} fbrowser; } smenu;
static bool nor_replace_games,open_ok,close_ok,wipe_ok;
static unsigned writes,wipes,opens,closes,source_size;
typedef int FIL;
static int f_open(FIL *fd,const char *path,int flags) { (void)fd;(void)path;(void)flags;opens++;return open_ok?0:1; }
static unsigned f_size(FIL *fd) {(void)fd;return source_size;}
static int f_close(FIL *fd) {(void)fd;closes++;return close_ok?0:1;}
static bool flashmgr_wipe(unsigned addr,unsigned size) {(void)addr;(void)size;wipes++;if(wipe_ok){smenu.fbrowser.freeblks=31;smenu.fbrowser.maxentries=0;}return wipe_ok;}
static void flashbrowser_reload(void) {}
#include "nor-actions.h"
static void reset(unsigned size,unsigned free,unsigned count) {
 memset(&spop,0,sizeof(spop));spop.p.norwr.i.romfs=size;source_size=size;
 smenu.fbrowser.freeblks=free;smenu.fbrowser.maxentries=count;
 writes=wipes=opens=closes=0;open_ok=close_ok=wipe_ok=true;
}
int main(void) {
 reset(8*1024*1024,31,0);request_nor_write();assert(!nor_replace_games&&spop.qpop.option==0&&!spop.qpop.clear_popup_ok);
 spop.p.norwr.i.use_dsaving=spop.p.norwr.i.ingame_menu_enabled=true;
 spop.qpop.callback(false);assert(!writes&&!wipes&&!opens);
 spop.qpop.callback(true);assert(writes==1&&!wipes&&opens==closes);
 assert(spop.p.norwr.i.use_dsaving&&spop.p.norwr.i.ingame_menu_enabled);
 reset(8*1024*1024,2,4);request_nor_write();assert(!nor_replace_games);spop.qpop.callback(true);assert(writes==1&&!wipes);
 reset(8*1024*1024+1,2,4);request_nor_write();assert(nor_replace_games&&strstr(spop.qpop.message,"Erase"));spop.qpop.callback(false);assert(!wipes&&!writes);spop.qpop.callback(true);assert(wipes==1&&writes==1);
 reset(MAX_GBA_ROM_SIZE,8,1);request_nor_write();spop.qpop.callback(true);assert(writes==1&&!wipes);
 reset(MAX_GBA_ROM_SIZE+1,31,0);request_nor_write();assert(spop.alert_msg&&!spop.qpop.message&&!writes&&!wipes);
 reset(UINT32_MAX,31,0);request_nor_write();assert(spop.alert_msg&&!spop.qpop.message);
 reset(0,31,0);request_nor_write();assert(spop.alert_msg&&!spop.qpop.message);
 reset(1024,31,32);request_nor_write();assert(nor_replace_games);wipe_ok=false;spop.qpop.callback(true);assert(wipes==1&&!writes&&spop.alert_msg);
 reset(1024,0,10);request_nor_write();source_size--;spop.qpop.callback(true);assert(!wipes&&!writes&&opens==closes&&spop.alert_msg);
 reset(1024,0,10);request_nor_write();open_ok=false;spop.qpop.callback(true);assert(!wipes&&!writes&&!closes&&spop.alert_msg);
 reset(1024,0,10);request_nor_write();close_ok=false;spop.qpop.callback(true);assert(!wipes&&!writes&&spop.alert_msg);
 reset(1024,1,0);request_nor_write();smenu.fbrowser.freeblks=0;spop.qpop.callback(true);assert(!wipes&&!writes&&spop.alert_msg);
 puts("NOR launch: capacity, cancel, confirmation, replace, maximum size and source/erase failure paths passed (mock flash).");
}
