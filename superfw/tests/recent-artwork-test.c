/* Exercise actual menu routing; artwork decoding has separate bus/fault tests. */
#include <assert.h>
#include <string.h>
#include "recent.h"
#include "flash_mgr.h"
#include "artwork_file.h"
#if TEST_NOR
#define SUPPORT_NORGAMES
#endif
static struct {
 uint16_t artwork_pixels[ARTWORK_MAX_PIXELS/2];
 uint8_t artwork_bmp_buffer[ARTWORK_BMP_MAX_SIZE];
 struct {unsigned gamecnt;t_flash_game_entry games[2];} nordata;
} storage,*sdr_state=&storage;
static artwork_info preview_info;
static unsigned artwork_mode=1,sd_calls,named_calls,code_calls;
static bool named_ok,code_ok;
#define PARAMS const char *path,unsigned style,uint8_t *pixels,size_t capacity,uint8_t *bmp,size_t bmp_capacity,artwork_info *info
#define CHECK() do {(void)path;assert(style==artwork_mode-1);assert(pixels==(void*)storage.artwork_pixels);assert(capacity==ARTWORK_MAX_PIXELS);assert(bmp==storage.artwork_bmp_buffer);assert(bmp_capacity==sizeof(storage.artwork_bmp_buffer));assert(info==&preview_info);}while(0)
bool artwork_load(PARAMS) {CHECK();++sd_calls;return named_ok;}
bool artwork_load_named(PARAMS) {CHECK();++named_calls;return named_ok;}
bool artwork_load_gamecode(const char code[4],uint8_t *pixels,size_t capacity,uint8_t *bmp,size_t bmp_capacity,artwork_info *info) {
 const char *path=code;unsigned style=artwork_mode-1;CHECK();assert(!memcmp(code,"BPRE",4));++code_calls;return code_ok;
}
#include "recent-art.h"
int main(void) {
 t_rentry entry={.fpath="/Game.gba"};
 named_ok=true;assert(recent_artwork_load(&entry));assert(sd_calls==1 && !named_calls && !code_calls);
 entry.flags=FLAG_RECENT_NOR;
 assert(recent_artwork_load(&entry));assert(named_calls==1 && !code_calls);
 named_ok=false;code_ok=true;storage.nordata.gamecnt=2;
 strcpy(storage.nordata.games[0].game_name,"/Other.gba");
 strcpy(storage.nordata.games[1].game_name,entry.fpath);
 memcpy(&storage.nordata.games[1].gamecode,"BPRE",4);
 assert(recent_artwork_load(&entry)==!!TEST_NOR);assert(code_calls==TEST_NOR);
 unsigned old=code_calls;artwork_mode=2;assert(!recent_artwork_load(&entry));assert(code_calls==old);
 artwork_mode=1;strcpy(entry.fpath,"/Missing.gba");assert(!recent_artwork_load(&entry));assert(code_calls==old);
 assert(sd_calls==1);
 return 0;
}
