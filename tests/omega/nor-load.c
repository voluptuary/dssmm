#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ff.h"
typedef uint32_t u32;typedef uint16_t u16;typedef uint8_t u8;typedef volatile uint16_t vu16;
#define MAX_NOR 64
#define MAX_pReadCache_size 0x20000
#define NOR_WRITE_FILE_ERROR 3
#define NOR_BLOCK_SIZE 0x20000U
#define NOR_CAPACITY_BYTES 0x4000000U
static uint8_t flash[0x600000];
#define FlashBase_S98 ((uintptr_t)flash)
typedef struct {char filename[100];u16 rompage,have_patch,have_RTS,reserved;u32 filesize,reserved2;char gamename[16];} FM_NOR_FS;
static FM_NOR_FS pNorFS[MAX_NOR];
static u8 pReadCache[MAX_pReadCache_size];
static FIL gfile;
static u32 game_total_NOR,iTrimSize;
static u16 gl_rts_on,gl_reset_on,gl_sleep_on,gl_cheat_on;
static char *gl_writing="Writing";
static u16 gImage_MENU[168*110];
static u32 gl_color_text;
#define NOR_MENU_WIDTH 168
static void Launcher_DrawNORWriteBackground(void){}
static void DrawPic(u16 *p,int x,int y,int w,int h,int a,int b,int c){}
static u16 DrawText12VisibleLength(char *s){return strlen(s);}
static void expect(int ok,const char *msg);
static void NOR_DrawWriteProgress(u32 done,u32 total,u32 *last){expect(total>0 && done<=total,"progress bounded");*last=done*100/total;}
static FSIZE_t input_size;
static int open_failure,close_failure,seek_failure,short_call,read_failure,reads,closes,erases,writes,publishes;
static u16 chip=0x223d;
static void expect(int ok,const char *msg){if(!ok){fprintf(stderr,"FAIL: %s\n",msg);exit(1);}}
FRESULT f_open(FIL *f,const TCHAR *name,BYTE mode){if(open_failure)return FR_NO_FILE;memset(f,0,sizeof(*f));f->obj.objsize=input_size;return FR_OK;}
FRESULT f_lseek(FIL *f,FSIZE_t p){if(seek_failure)return FR_DISK_ERR;f->fptr=p;return FR_OK;}
FRESULT f_read(FIL *f,void *out,UINT n,UINT *got){reads++;if(reads==read_failure){*got=0;return FR_DISK_ERR;}FSIZE_t left=f_size(f)-f->fptr;*got=left<n?(UINT)left:n;if(reads==short_call&&*got)(*got)--;memset(out,0x5a,*got);f->fptr+=*got;return FR_OK;}
FRESULT f_close(FIL *f){closes++;return close_failure?FR_DISK_ERR:FR_OK;}
static u16 Read_S98NOR_ID(void){return chip;}
static void dmaCopy(const void *a,void *b,u32 n){memcpy(b,a,n);}
static void DrawHZText12(char *s,int a,int b,int c,int d,int e){}
static void Block_Erase(u32 address){erases++;}
static void WriteFlash_with32word(u32 address,u8 *p,u32 n){writes++;expect(n==NOR_BLOCK_SIZE,"fixed write block");}
static void PatchInternal(u32 *p,int n,u32 b){}
static void GBApatch_NOR(u32 *p,int n,u32 b){}
static void GBApatch_Cleanrom_NOR(u32 *p,u32 b){}
static void Save_NOR_info(u16 *p,u32 n){publishes++;}
#include "omega-nor.impl.h"
static void reset(void){input_size=0x20001;open_failure=close_failure=seek_failure=short_call=read_failure=reads=closes=erases=writes=publishes=0;game_total_NOR=iTrimSize=0;chip=0x223d;memset(pNorFS,0x33,sizeof(pNorFS));memset(pReadCache,0xa7,sizeof(pReadCache));}
int main(void){
 FM_NOR_FS prior;u32 r;
 reset();open_failure=1;r=Loadfile2NOR("Test.gba",0,0);expect(r!=0,"open failure must not report success");expect(!erases&&!writes&&!publishes,"open failure has no flash writes");
 reset();prior=pNorFS[0];short_call=2;expect(Loadfile2NOR("Test.gba",0,0)!=0,"short data read fails");expect(!erases&&!writes&&!publishes,"short first block is not erased/programmed");expect(!memcmp(&prior,&pNorFS[0],sizeof(prior)),"catalog unchanged on failed input");
 reset();short_call=1;expect(Loadfile2NOR("Test.gba",0,0)!=0&&!erases,"short header fails before erasing");
 reset();read_failure=3;expect(Loadfile2NOR("Test.gba",0,0)!=0,"later read failure reported");expect(erases==1&&writes==1&&!publishes,"later read failure stops before next erase/catalog");
 reset();seek_failure=1;expect(Loadfile2NOR("Test.gba",0,0)!=0&&!erases,"seek failure aborts");
 reset();close_failure=1;prior=pNorFS[0];expect(Loadfile2NOR("Test.gba",0,0)!=0&&!publishes,"close failure blocks catalog");expect(!memcmp(&prior,&pNorFS[0],sizeof(prior)),"RAM catalog unchanged on close failure");
 reset();expect(Loadfile2NOR("Test.gba",0,0)==0,"valid partial final block succeeds");expect(writes==2&&publishes==1&&closes==1,"successful write/close/catalog counts");expect(pReadCache[0]==0x5a&&pReadCache[1]==0xff&&pReadCache[NOR_BLOCK_SIZE-1]==0xff,"unused final block is FF padded");
 reset();input_size=((FSIZE_t)1<<32)+0x20000;expect(Loadfile2NOR("Test.gba",0,0)!=0&&!erases,"64-bit oversize rejected before narrowing");
 reset();expect(Loadfile2NOR("Test.gba",NOR_CAPACITY_BYTES+NOR_BLOCK_SIZE,0)!=0&&!erases,"offset past capacity rejected");
 reset();game_total_NOR=MAX_NOR;expect(Loadfile2NOR("Test.gba",0,0)!=0&&!erases,"catalog capacity enforced");
 reset();input_size=NOR_BLOCK_SIZE;expect(Loadfile2NOR("Test.gba",NOR_CAPACITY_BYTES-NOR_BLOCK_SIZE,0)==0&&writes==1,"last aligned block fits exactly");
 reset();input_size=NOR_BLOCK_SIZE;iTrimSize=NOR_BLOCK_SIZE;expect(Loadfile2NOR("Test.gba",NOR_CAPACITY_BYTES-NOR_BLOCK_SIZE,1)==2&&!erases,"extra patch block capacity checked");
 reset();iTrimSize=0x40000;expect(Loadfile2NOR("Test.gba",0,1)==0&&writes==3&&publishes==1,"extra patch block retained");
 reset();expect(Loadfile2NOR("Test.gba",1,0)!=0&&!erases,"misaligned target rejected");
 reset();input_size=0;expect(Loadfile2NOR("Test.gba",0,0)!=0&&!erases,"empty ROM rejected");
 reset();chip=0;expect(Loadfile2NOR("Test.gba",0,0)!=0&&!erases,"unknown chip rejected");
 puts("Omega NOR load: I/O failures, capacity, padding and catalog publication passed (flash mocked).");
}
