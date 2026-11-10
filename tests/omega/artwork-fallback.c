#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
typedef uint32_t u32;
typedef unsigned char u8;
typedef unsigned int UINT;
typedef char TCHAR;
#define MAX_path_len 256
#define FA_READ 1
#define EWRAM_BSS
#define AM_DIR 16
#include "launcher_artwork_path.h"
typedef struct {char fname[256];unsigned char fattrib;} FILINFO;
static char currentpath[256]="/";
static int missing_mirror, failed_opens;
static int f_stat(const char *path,FILINFO *info){
 memset(info,0,sizeof(*info));
 if(!strcmp(path,"/GAMES~1")){strcpy(info->fname,"Game Collection");info->fattrib=AM_DIR;return 0;}
 if(!strcmp(path,"/GAMES~1/SONIC~1.gg")){strcpy(info->fname,"Sonic The Hedgehog.gg");return 0;}
 return 1;
}
#define FR_OK 0
static int gfile, opened, closed, stopped, custom, header, header_calls;
static u32 read_result, close_result;
static UINT read_count=64;
static char seen[400];
static const char *Launcher_ThumbnailFolder(void){return "/SYSTEM/IMGS";}
static u32 Launcher_ThumbnailReadSize(void){return 64;}
static int f_open(int *f,const char *p,int m){(void)f;(void)m;opened++;snprintf(seen,sizeof(seen),"%s",p);if(missing_mirror && strstr(p,"Game Collection")){failed_opens++;return 1;}return 0;}
static int f_close(int *f){(void)f;closed++;return close_result;}
static void UIAudio_StopForSharedBufferUse(void){stopped++;}
static u32 Launcher_ReadThumbnailBytes(int *f,u8 *b,u32 n,UINT *got){(void)f;(void)b;(void)n;*got=read_count;return read_result;}
static void Launcher_CustomThumbFileName(const char *f,char *n,u32 z){(void)f;if(z)n[0]=0;}
static u32 Launcher_LoadCustomThumbnailByName(const char *n,u8 *d){(void)n;(void)d;return custom;}
static u32 Launcher_LoadThumbnailFromRomHeader(char *n,u8 *d){(void)n;(void)d;header_calls++;return header;}
#include "omega-artwork.impl.h"
int main(void){
 u8 dst[64];char long_name[400];
 assert(Load_ThumbnailEx("Sonic.gg",dst));assert(!strcmp(seen,"/SYSTEM/IMGS/Sonic.bmp"));assert(header_calls==0);
 assert(Load_ThumbnailEx("/Games/Long name.nes",dst));assert(!strcmp(seen,"/SYSTEM/IMGS/Long name.bmp"));
 assert(Load_ThumbnailEx("C:\\Games\\Color.GBC",dst));assert(!strcmp(seen,"/SYSTEM/IMGS/Color.bmp"));
 custom=1;opened=closed=stopped=0;assert(Load_ThumbnailEx("Game.gba",dst));assert(!opened);custom=0;
 header=1;assert(Load_ThumbnailEx("Game.GBA",dst));assert(header_calls==1 && !opened);header=0;
 assert(Load_ThumbnailEx("Game.gba",dst));assert(header_calls==2 && opened==1);
 read_count=63;assert(!Launcher_LoadFilenameThumbnail("Game.nes",dst));read_count=64;
 read_result=1;assert(!Launcher_LoadFilenameThumbnail("Game.nes",dst));read_result=0;
 close_result=1;assert(!Launcher_LoadFilenameThumbnail("Game.nes",dst));close_result=0;
 assert(!Launcher_LoadFilenameThumbnail(NULL,dst));assert(!Launcher_LoadFilenameThumbnail(".nes",dst));
 memset(long_name,'A',sizeof(long_name));memcpy(long_name+390,".nes",5);assert(!Launcher_LoadFilenameThumbnail(long_name,dst));
 strcpy(currentpath,"/GAMES~1");
 assert(Load_ThumbnailEx("/GAMES~1/SONIC~1.gg",dst));assert(!strcmp(seen,"/SYSTEM/IMGS/Game Collection/Sonic The Hedgehog.bmp"));
 missing_mirror=1;assert(Load_ThumbnailEx("/GAMES~1/SONIC~1.gg",dst));assert(!strcmp(seen,"/SYSTEM/IMGS/Sonic The Hedgehog.bmp"));
 assert(opened==closed+failed_opens && stopped==closed);
 puts("Omega filename artwork: precedence, all-system paths, length bounds and I/O failures passed.");
}
