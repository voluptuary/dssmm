/* Source routing for the shared renderer; no hardware driver is mocked here. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "config.h"
#include "flash_mgr.h"
#include "artwork_file.h"
#if TEST_NOR
#define SUPPORT_NORGAMES
#endif
#define MENUTAB_ROMBROWSE 1
#define MENUTAB_NORBROWSE 2
#define AM_DIR 16
#define ICON_GBACART 77
#define npf_snprintf snprintf
#define strbcpy(out,in,n) snprintf(out,n,"%s",in)
struct entry {unsigned attr;const char *fname;};
static struct {unsigned menu_tab;struct {unsigned dispentries;int selector;const char *cpath;} browser;struct {unsigned maxentries;int selector;} fbrowser;} smenu;
static struct {struct entry *filefilter[3];struct {t_flash_game_entry games[2];} nordata;uint8_t artwork_bmp_buffer[ARTWORK_BMP_MAX_SIZE];} storage,*sdr_state=&storage;
static unsigned artwork_mode=1,sd_calls,dir_calls,named_calls,code_calls;
static bool named_ok;
static unsigned browser_entry_icon(const struct entry *e) {return e->attr&AM_DIR?40:41;}
#define PARAMS const char *path,unsigned style,uint8_t *pixels,size_t capacity,uint8_t *bmp,size_t bmp_capacity,artwork_info *info
#define CHECK() do {assert(path && pixels && info);assert(style==artwork_mode-1);assert(capacity==ARTWORK_MAX_PIXELS);assert(bmp==storage.artwork_bmp_buffer);assert(bmp_capacity==sizeof(storage.artwork_bmp_buffer));}while(0)
bool artwork_load(PARAMS) {CHECK();++sd_calls;return true;}
bool artwork_load_directory(PARAMS) {CHECK();++dir_calls;return true;}
bool artwork_load_named(PARAMS) {CHECK();++named_calls;return named_ok;}
bool artwork_load_gamecode(const char code[4],uint8_t *pixels,size_t capacity,uint8_t *bmp,size_t bmp_capacity,artwork_info *info) {
 const char *path=code;unsigned style=artwork_mode-1;CHECK();assert(!memcmp(code,"BPRE",4));++code_calls;return true;
}
#include "carousel-source.h"
int main(void) {
 struct entry entry={.fname="Game.gba"};storage.filefilter[0]=&entry;
 smenu.menu_tab=MENUTAB_ROMBROWSE;smenu.browser.dispentries=1;smenu.browser.cpath="/Games/";
 assert(carousel_source_active() && !carousel_source_nor() && carousel_source_count()==1 && carousel_source_selected()==0);
 char path[256];assert(carousel_entry_path(0,path,sizeof(path)));assert(!strcmp(path,"/Games/Game.gba"));
 assert(!carousel_entry_path(0,path,4));assert(!strcmp(carousel_entry_name(0),"Game.gba"));assert(carousel_entry_icon(0)==41 && !carousel_entry_directory(0));
 uint8_t pixels[2];artwork_info info;
 assert(carousel_entry_art(0,"/Games/Game.gba",pixels,&info));assert(sd_calls==1 && !dir_calls);
 entry.attr=AM_DIR;assert(carousel_entry_directory(0) && carousel_entry_icon(0)==40);
 assert(carousel_entry_art(0,"/Games/Folder",pixels,&info));assert(dir_calls==1);
#if TEST_NOR
 smenu.menu_tab=MENUTAB_NORBROWSE;smenu.fbrowser.maxentries=2;smenu.fbrowser.selector=1;
 t_flash_game_entry *game=&storage.nordata.games[0];strcpy(game->game_name,"/Old/Game.gba");game->bnoffset=5;memcpy(&game->gamecode,"BPRE",4);
 assert(carousel_source_nor() && carousel_source_active() && carousel_source_count()==2 && carousel_source_selected()==1);
 assert(carousel_entry_path(0,path,sizeof(path)) && !strcmp(path,game->game_name));assert(!carousel_entry_path(0,path,4));
 assert(!strcmp(carousel_entry_name(0),"Game.gba") && carousel_entry_icon(0)==ICON_GBACART && !carousel_entry_directory(0));
 named_ok=true;assert(carousel_entry_art(0,path,pixels,&info));assert(named_calls==1 && !code_calls);
 named_ok=false;assert(carousel_entry_art(0,path,pixels,&info));assert(code_calls==1);
 artwork_mode=2;assert(!carousel_entry_art(0,path,pixels,&info));assert(code_calls==1);
#endif
 smenu.menu_tab=7;assert(!carousel_source_active());
 return 0;
}
