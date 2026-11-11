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
#define MAX_NOR 64
#define LAUNCHER_THUMB_BMP_HEADER 54
#define FA_READ 1
#define EWRAM_BSS
#define AM_DIR 16
#include "launcher_artwork_path.h"
typedef struct {char fname[256];unsigned char fattrib;} FILINFO;
static char currentpath[256]="/";
static int missing_mirror, failed_opens;
static unsigned stats;
static int f_stat(const char *path,FILINFO *info){
 stats++;
 memset(info,0,sizeof(*info));
 if(!strcmp(path,"/GAMES~1")){strcpy(info->fname,"Game Collection");info->fattrib=AM_DIR;return 0;}
 if(!strcmp(path,"/GAMES~1/SONIC~1.gg") || !strcmp(path,"SONIC~1.gg")){strcpy(info->fname,"Sonic The Hedgehog.gg");return 0;}
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
static u32 Launcher_LoadThumbnailFromRomHeader(const char *n,u8 *d){(void)n;(void)d;header_calls++;return header;}
static unsigned nor_page,recents_view_active,folder_total,game_total_SD=1;
static char p_recently_play[10][512];
static unsigned char launcher_virtual_gamecode_valid[10];
static unsigned char launcher_virtual_gamecode[10][4];
static struct {char filename[100];char gamename[16];} pNorFS[64];
typedef struct {char filename[100];char altname[13];} MockRecord;
static MockRecord pFolder[96],pFilename_buffer[512];
static const char *Launcher_FileIdentityName(const MockRecord *record){return record->altname[0]?record->altname:record->filename;}
static const char *Launcher_FolderIdentityName(const MockRecord *record){return record->altname[0]?record->altname:record->filename;}
static unsigned Launcher_GetTotalEntries(void){return nor_page?1:folder_total+game_total_SD;}
static unsigned Launcher_IsNORPage(void){return nor_page;}
static unsigned Launcher_IsValidGameCodeBytes(const u8 *p){(void)p;return 1;}
static unsigned Launcher_LoadThumbnailByGameCode(const u8 *p,u8 *d){(void)p;(void)d;return header;}
static const char *nor_source;
static char captured_source[514];
static const char *Omega_ReadNorArtworkPath(unsigned slot){assert(slot<64);return nor_source;}
static void Omega_SaveNorArtworkPath(unsigned slot,const char *path){assert(slot<64);snprintf(captured_source,sizeof captured_source,"%s",path);}
#include "omega-artwork.impl.h"
int main(void){
 u8 dst[64];char long_name[400];
 assert(Load_ThumbnailEx("Sonic.gg",dst));assert(!strcmp(seen,"/SYSTEM/IMGS/Sonic.bmp"));assert(header_calls==0);
 assert(Load_ThumbnailEx("/Games/Long name.nes",dst));assert(!strcmp(seen,"/SYSTEM/IMGS/Long name.bmp"));
 assert(Load_ThumbnailEx("C:\\Games\\Color.GBC",dst));assert(!strcmp(seen,"/SYSTEM/IMGS/Color.bmp"));
 custom=1;opened=closed=stopped=0;assert(Load_ThumbnailEx("Game.gba",dst));assert(!opened);custom=0;
 header=1;assert(Load_ThumbnailEx("Game.GBA",dst));assert(header_calls==1 && !opened);header=0;
 assert(Load_ThumbnailEx("Game.gba",dst));assert(header_calls==2 && opened==1);
 assert(Load_ThumbnailEx("Game.agb",dst));assert(header_calls==3);
 read_count=63;assert(!Launcher_LoadFilenameThumbnail("Game.nes",dst));read_count=64;
 read_result=1;assert(!Launcher_LoadFilenameThumbnail("Game.nes",dst));read_result=0;
 close_result=1;assert(!Launcher_LoadFilenameThumbnail("Game.nes",dst));close_result=0;
 assert(!Launcher_LoadFilenameThumbnail(NULL,dst));assert(!Launcher_LoadFilenameThumbnail(".nes",dst));
 memset(long_name,'A',sizeof(long_name));memcpy(long_name+390,".nes",5);assert(!Launcher_LoadFilenameThumbnail(long_name,dst));
 strcpy(currentpath,"/GAMES~1");
 assert(Load_ThumbnailEx("/GAMES~1/SONIC~1.gg",dst));assert(!strcmp(seen,"/SYSTEM/IMGS/Game Collection/Sonic The Hedgehog.bmp"));
 missing_mirror=1;assert(Load_ThumbnailEx("/GAMES~1/SONIC~1.gg",dst));assert(!strcmp(seen,"/SYSTEM/IMGS/Sonic The Hedgehog.bmp"));
 assert(opened==closed+failed_opens && stopped==closed);
 strcpy(currentpath,"/");header=0;custom=0;missing_mirror=0;
 strcpy(pFilename_buffer[0].filename,"Sonic.gg");u8 image[54+64];
 assert(Launcher_LoadThumbDataForIndex(0,image+54));assert(!strcmp(seen,"/SYSTEM/IMGS/Sonic.bmp"));
 nor_page=1;strcpy(pNorFS[0].filename,"Sonic.gba");unsigned prior_stats=stats;
 assert(Launcher_LoadThumbDataForIndex(0,image+54));assert(!strcmp(seen,"/SYSTEM/IMGS/Sonic.bmp"));
 assert(stats==prior_stats);
 Launcher_SaveNorArtworkSource(0,"/GAMES~1","SONIC~1.gg");
 assert(!strcmp(captured_source,"/Game Collection/Sonic The Hedgehog.gg"));
 nor_source="/GAMES~1/Sonic The Hedgehog.gg";
 assert(Launcher_LoadThumbDataForIndex(0,image+54));
 assert(!strcmp(seen,"/SYSTEM/IMGS/Game Collection/Sonic The Hedgehog.bmp"));
 nor_source=NULL;
 strcpy(currentpath,"/Recently Played");missing_mirror=0;
 assert(Launcher_LoadOmegaThumbnailForDirectory("SONIC~1.gg","/GAMES~1",dst));assert(!strcmp(seen,"/SYSTEM/IMGS/Game Collection/Sonic The Hedgehog.bmp"));
 nor_page=0;recents_view_active=1;strcpy(currentpath,"/Recently Played");strcpy(p_recently_play[0],"/GAMES~1/SONIC~1.gg");
 assert(Launcher_LoadThumbDataForIndex(0,image+54));assert(!strcmp(seen,"/SYSTEM/IMGS/Game Collection/Sonic The Hedgehog.bmp"));
 puts("Omega filename artwork: precedence, all-system paths, length bounds and I/O failures passed.");
}
