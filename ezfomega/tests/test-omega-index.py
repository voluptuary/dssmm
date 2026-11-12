#!/usr/bin/env python3
"""Production indexed records, paired sorting and validated header-art fast opens."""
from pathlib import Path
import re,subprocess,tempfile,os
root=Path(__file__).resolve().parents[1]
s=(root/'source/ezkernelnew.c').read_text()
def function(name):
 m=re.search(r'^[^\n;{}]*\b'+name+r'\([^;{}]*?\)\s*\{',s,re.M);assert m,name
 return s[m.start():s.index('\n}',m.start())+2]
h=(root/'source/ezkernel.h').read_text();records=h[h.index('typedef struct FM_Folder_SECT'):h.index('typedef enum')]
pre=r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
typedef uint32_t u32;typedef uint8_t u8;typedef char TCHAR;typedef unsigned UINT;
#define MAX_files 512
#define MAX_folder 96
#define LAUNCHER_FILE_DIRENT_INVALID 0xFFFFFFFFu
#define FR_OK 0
#define FR_NO_FILE 4
#define FA_READ 1
'''
mid=r'''
static FM_FILE_FS pFilename_buffer[512],pFilename_temp,pFilename_sort_probe;
static FM_Folder_FS pFolder[96],launcher_folder_sort_temp;
static u32 pFilename_dirent_offset[512];
static unsigned game_total_SD,recents_view_active,nor;
static char currentpath[256]="/Games";static unsigned char GAMECODE[4];
static int gfile,fast,normal,failed_fast,bad_seek,bad_read,bad_close;
static unsigned expected_offset,expected_size;
static unsigned Launcher_IsNORPage(void){return nor;}
static void dmaCopy(const void *src,void *dst,unsigned size){
 uintptr_t a=(uintptr_t)src,b=(uintptr_t)dst,base=(uintptr_t)pFilename_buffer;
 assert(!(a>=base && a<base+sizeof pFilename_buffer && b>=base && b<base+sizeof pFilename_buffer));
 memcpy(dst,src,size);
}
static int f_open_by_dirent(int *f,const char *dir,u32 off,const char *name,u32 size){
 (void)f;assert(!strcmp(dir,"/Games"));assert(!strcmp(name,"LONGGA~1.GBA"));
 assert(off==expected_offset && size==expected_size);fast++;return failed_fast;
}
static int f_open(int *f,const char *name,int mode){(void)f;(void)name;(void)mode;normal++;return 0;}
static int f_lseek(int *f,unsigned offset){(void)f;assert(offset==0xAC);return bad_seek;}
static int f_close(int *f){(void)f;return bad_close;}
static unsigned Launcher_ReadThumbnailBytes(int *f,u8 *dst,unsigned size,UINT *out){
 (void)f;assert(size==4);memcpy(dst,"ABCD",4);*out=4;return bad_read;
}
static unsigned Launcher_IsValidGameCodeBytes(const u8 *code){return !memcmp(code,"ABCD",4);}
static unsigned Launcher_LoadThumbnailByGameCode(const u8 *code,u8 *dst){(void)dst;return !memcmp(code,"ABCD",4);}
'''
post=r'''
int main(void){
 char longname[200];memset(longname,'x',sizeof longname);strcpy(longname+190,".gba");
 Launcher_StoreFileRecord(0,longname,"LONGGA~1.GBA",1234);
 assert(strlen(pFilename_buffer[0].filename)==99 && pFilename_buffer[0].flags==FM_ENTRY_ALIAS_DISPLAY);
 assert(!strcmp(Launcher_FileIdentityName(&pFilename_buffer[0]),"LONGGA~1.GBA"));
 Launcher_StoreFolderRecord(0,longname,"LONGFO~1");assert(!strcmp(Launcher_FolderIdentityName(&pFolder[0]),"LONGFO~1"));
 game_total_SD=512;
 for(unsigned i=0;i<512;i++){
  char name[50];snprintf(name,sizeof name,"%04u.gba",511-i);
  Launcher_StoreFileRecord(i,name,"",i);pFilename_dirent_offset[i]=i*32;
 }
 Sort_file(512);
 for(unsigned i=0;i<512;i++)assert(pFilename_buffer[i].filesize==511-i && pFilename_dirent_offset[i]==(511-i)*32);
 strcpy(pFolder[0].filename,"z");strcpy(pFolder[1].filename,"a");Sort_folder(2);assert(!strcmp(pFolder[0].filename,"a"));
 Launcher_StoreFileRecord(0,longname,"LONGGA~1.GBA",1234);
 pFilename_dirent_offset[0]=expected_offset=96;expected_size=1234;game_total_SD=1;
 unsigned char dst[8];const char *identity=Launcher_FileIdentityName(&pFilename_buffer[0]);
 assert(Launcher_LoadThumbnailFromRomHeader(identity,dst) && fast==1 && !normal);
 failed_fast=1;assert(Launcher_LoadThumbnailFromRomHeader(identity,dst) && fast==2 && normal==1);failed_fast=0;
 bad_seek=1;assert(!Launcher_LoadThumbnailFromRomHeader(identity,dst));bad_seek=0;
 bad_read=1;assert(!Launcher_LoadThumbnailFromRomHeader(identity,dst));bad_read=0;
 bad_close=1;assert(!Launcher_LoadThumbnailFromRomHeader(identity,dst));bad_close=0;
 pFilename_dirent_offset[0]=LAUNCHER_FILE_DIRENT_INVALID;fast=normal=0;
 assert(Launcher_LoadThumbnailFromRomHeader(identity,dst) && !fast && normal==1);
 puts("Omega indexed identities, 512 paired offsets, staged sort and header fast-open/fallback/error paths passed.");
}
'''
names=['Launcher_IsCP936LeadByte','Launcher_CopyIndexedName','Launcher_StoreFileRecord','Launcher_FileIdentityName','Launcher_FolderIdentityName','Launcher_StoreFolderRecord','Sort_folder','Sort_file','Launcher_LoadThumbnailFromRomHeader']
with tempfile.TemporaryDirectory() as folder:
 c=Path(folder)/'test.c';exe=Path(folder)/'test';c.write_text(pre+records+mid+'\n'.join(function(n) for n in names)+post)
 subprocess.run([os.environ.get('CC','cc'),'-std=c99','-O1','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-pie','-no-pie',str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
