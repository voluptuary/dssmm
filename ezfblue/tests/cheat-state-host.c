#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef unsigned UINT;
typedef char TCHAR;
typedef int FRESULT;
typedef struct { unsigned size, pos; } FIL;
#define EWRAM_BSS
#define MAX_path_len 256
#define MAX_pReadCache_size 0x20000
#define MAX_KEY_LEN 50
#define FR_OK 0
#define FR_EXIST 1
#define FA_READ 1
#define FA_WRITE 2
#define FA_CREATE_ALWAYS 4
typedef struct { char LINEname[50]; u8 is_section, section_val_count, len, select; } FM_CHT_LINE;
static FM_CHT_LINE rows[200];
static u8 *pCHTbuffer = (u8*)rows;
static char buf[6000];
TCHAR currentpath[256] = "/GBA";
u16 gl_cheat_remember;
static u32 gl_cheat_selected_count;
static FIL gfile = {4,0};
static u8 disk[4096];
static unsigned disk_size;
static int short_read, short_write, read_error, write_error, close_error, open_error;
static char last_path[64];
static const char content[] = "abcd";
static unsigned f_size(FIL *f) { return f->size; }
static int f_lseek(FIL *f, unsigned pos) { f->pos = pos; return 0; }
static int f_open(FIL *f, const char *path, unsigned mode) {
 if(open_error) return 2;
 strcpy(last_path,path); f->pos=0;
 if(mode & FA_WRITE) disk_size=0;
 f->size=disk_size; return 0;
}
static int f_read(FIL *f, void *out, unsigned size, unsigned *n) {
 if(read_error) return 2;
 unsigned available = f == &gfile ? sizeof(content)-1 : disk_size;
 *n = size > available-f->pos ? available-f->pos : size;
 if(short_read && *n) --*n;
 memcpy(out,(f == &gfile ? (const u8*)content : disk)+f->pos,*n);
 f->pos+=*n; return 0;
}
static int f_write(FIL *f, const void *in, unsigned size, unsigned *n) {
 (void)f; if(write_error) return 2;
 *n=size-(short_write ? 1 : 0); assert(*n <= sizeof(disk));
 memcpy(disk,in,*n); disk_size=*n; return 0;
}
static int f_close(FIL *f) { (void)f; return close_error ? 2 : 0; }
static int f_mkdir(const char *p) { (void)p; return FR_EXIST; }
static unsigned cleared, drawn;
static unsigned gl_color_selected, gl_color_text, gl_color_selectBG_sd;
static void Launcher_ClearCheatRegion(unsigned x,unsigned y,unsigned w,unsigned h) {
 (void)x;(void)y;(void)w;(void)h; cleared++;
}
#define Clear(x,y,w,h,c,d) ((void)(x),(void)(y),(void)(w),(void)(h),(void)(c),(void)(d))
#define Draw_select_icon(...) ((void)0)
static unsigned CheatTextVisibleColumns(const char *s) { return strlen(s); }
static const char *Launcher_OnOffText(unsigned n) { return n ? "On" : "Off"; }
#define DrawCheatText12(s,l,x,y,c,d) ((void)(s),(void)(l),(void)(x),(void)(y),(void)(c),(void)(d),drawn++)
#include CHEAT_IMPL
static void reseal(void) {
 CheatState *s=(CheatState*)disk;
 s->checksum=CheatHash(2166136261u,s,offsetof(CheatState,checksum));
}
static void setup(void) {
 memset(rows,0,sizeof(rows));
 rows[0].is_section=rows[3].is_section=1;
 strcpy(rows[0].LINEname,"Group"); strcpy(rows[3].LINEname,"Second");
 CheatSelectionMakeKey(cheat_active_key,sizeof(cheat_active_key),"Test.gba",0xffff);
 CheatContentHash(); assert(cheat_hash_valid);
 rows[1].select=rows[4].select=1;
 CheatSelectionRemember(6); assert(cheat_selection_cache_count==2);
 assert(CheatStateSave()); rows[1].select=rows[4].select=0;
}
int main(void) {
 setup(); assert(CheatStateApply(6)==2); assert(rows[1].select && rows[4].select);
 CheatSelectionForget(); assert(!CheatSelectionAppliesTo("Test.gba",0xffff));
 setup(); CheatSelectionRestore(6); assert(gl_cheat_selected_count==2);
 char key[CHEAT_KEY_LEN]; strcpy(currentpath,"/Other");
 CheatSelectionMakeKey(key,sizeof(key),"Test.gba",0xffff);
 assert(strcmp(key,cheat_active_key)); strcpy(currentpath,"/GBA");
 char longname[256]; memset(longname,'a',255); longname[255]=0;
 CheatSelectionMakeKey(key,sizeof(key),longname,0xffff);
 longname[220]='b'; CheatSelectionMakeKey(cheat_active_key,sizeof(cheat_active_key),longname,0xffff);
 assert(strcmp(key,cheat_active_key));
 setup(); cheat_active_hash++; assert(CheatStateApply(6)==0); assert(!rows[1].select);
 setup(); disk[40]^=1; assert(!CheatStateApply(6));
 setup(); ((CheatState*)disk)->count=129; reseal(); assert(!CheatStateApply(6));
 setup(); ((CheatState*)disk)->indices[1]=200; reseal(); assert(!CheatStateApply(6)); assert(!rows[1].select);
 setup(); ((CheatState*)disk)->indices[1]=2; reseal(); assert(!CheatStateApply(6));
 setup(); ((CheatState*)disk)->indices[0]=0; reseal(); assert(!CheatStateApply(6));
 setup(); read_error=1; CheatContentHash(); assert(!cheat_hash_valid);
 CheatSelectionRestore(6); assert(!gl_cheat_selected_count && !rows[1].select);
 read_error=0;
 setup(); short_read=1; assert(!CheatStateApply(6)); short_read=0;
 setup(); close_error=1; assert(!CheatStateApply(6)); assert(!CheatStateSave()); close_error=0;
 setup(); short_write=1; assert(!CheatStateSave()); short_write=0; assert(!CheatStateApply(6));
 setup(); write_error=1; assert(!CheatStateSave()); write_error=0;
 setup(); open_error=1; assert(!CheatStateSave()); assert(!CheatStateApply(6)); open_error=0;
 setup(); rows[1].select=rows[4].select=0; CheatSelectionRemember(6);
 assert(CheatStateSave()); assert(CheatSelectionAppliesTo("Test.gba",0xffff)); assert(!CheatStateApply(6));
 /* Collapse a large group at the bottom, preserving selection in hidden rows. */
 memset(rows,0,sizeof(rows)); rows[0].is_section=rows[12].is_section=1; rows[18].select=1;
 assert(GetVisibleCheatMenuCount(20)==20); assert(SetCheatGroupExpanded(12,20,0));
 assert(GetVisibleCheatMenuCount(20)==13); assert(rows[18].select);
 u32 select=2,offset=10; ClampCheatViewport(13,&select,&offset);
 assert(offset==3 && select==9); assert(GetVisibleCheatMenuEntryIndex(20,12)==12);
 Show_KEY_val(20,13,select,offset); assert(cleared==10 && drawn==10);
 assert(SetCheatGroupExpanded(0,20,0)); ClampCheatViewport(2,&select,&offset);
 cleared=drawn=0; Show_KEY_val(20,2,select,offset); assert(cleared==10 && drawn==2);
 assert(offset==0 && select==1); assert(!SetCheatGroupExpanded(200,20,1));
 ClampCheatViewport(0,&select,&offset); assert(select==0 && offset==0);
 ToggleCheatMenuEntry(19,20); assert(rows[19].select && !rows[18].select);
 ToggleCheatMenuEntry(19,20); assert(!rows[19].select);
 ToggleCheatMenuEntry(200,20);
 assert(SetCheatGroupExpanded(12,20,1));
 assert(CheatVisibleSelectionMask(20,GetVisibleCheatMenuCount(20),0)==0);
 RedrawChangedCheatSelections(1,20,GetVisibleCheatMenuCount(20),0,0);
 puts("PASS: cheat persistence, failures, identity, bounded selections and collapsed viewport");
}
