#!/usr/bin/env python3
"""Exercise the production clipped-name resolver, including stale/error cases."""
from pathlib import Path
import os, subprocess, tempfile
root = Path(__file__).resolve().parents[1]
s = (root/'source/ezkernelnew.c').read_text()
a = s.index('static u32 Launcher_ResolveIndexedLaunchName(')
b = s.index('\nu8 SD_list_MENU(', a)
pre = r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
typedef unsigned u32; typedef uint32_t DWORD; typedef char TCHAR; typedef int FRESULT;
#define FR_OK 0
#define AM_DIR 16
#define FM_ENTRY_ALIAS_DISPLAY 1
#define LAUNCHER_FILE_DIRENT_INVALID 0xffffffffu
typedef struct {char filename[100]; unsigned filesize; char altname[13]; unsigned flags;} Record;
static Record records[2],pFilename_temp;
#define pFilename_buffer records
static unsigned game_total_SD=1,pFilename_dirent_offset[2]={32};
static char currentpath[]="/Games";
static int dir,cursor,open_error,read_error,close_error;
static struct {char fname[256];unsigned fsize,fattrib;} fileinfo,entry;
static void dmaCopy(const void *a,void *b,unsigned n){memcpy(b,a,n);}
static const char *Launcher_FileIdentityName(const Record *r){return r->altname[0]?r->altname:r->filename;}
static int f_opendir(int *d,const char *p){(void)d;assert(!strcmp(p,"/Games"));cursor=0;return open_error;}
static int f_readdir_with_offset(int *d,void *f,DWORD *o){(void)d;(void)f;*o=32;if(cursor++)fileinfo.fname[0]=0;else memcpy(&fileinfo,&entry,sizeof entry);return read_error;}
static int f_closedir(int *d){(void)d;return close_error;}
'''
post = r'''
int main(void){
 char out[256],full[256];memset(full,'A',sizeof full);strcpy(full+240,".gba");
 strcpy(records[0].filename,"Game (USA).gba");
 assert(Launcher_ResolveIndexedLaunchName(0,out,sizeof out)&&!strcmp(out,records[0].filename));
 strcpy(records[0].altname,"GAME~1.GBA");
 assert(Launcher_ResolveIndexedLaunchName(0,out,sizeof out)&&!strcmp(out,"GAME~1.GBA"));
 records[0].altname[0]=0;memcpy(records[0].filename,full,99);records[0].filename[99]=0;
 records[0].flags=FM_ENTRY_ALIAS_DISPLAY;records[0].filesize=entry.fsize=32768;strcpy(entry.fname,full);
 assert(Launcher_ResolveIndexedLaunchName(0,out,sizeof out)&&!strcmp(out,full));
 assert(!Launcher_ResolveIndexedLaunchName(0,out,100));
 assert(!Launcher_ResolveIndexedLaunchName(1,out,sizeof out));
 entry.fsize++;assert(!Launcher_ResolveIndexedLaunchName(0,out,sizeof out));entry.fsize--;
 entry.fattrib=AM_DIR;assert(!Launcher_ResolveIndexedLaunchName(0,out,sizeof out));entry.fattrib=0;
 entry.fname[0]='B';assert(!Launcher_ResolveIndexedLaunchName(0,out,sizeof out));entry.fname[0]='A';
 pFilename_dirent_offset[0]=64;assert(!Launcher_ResolveIndexedLaunchName(0,out,sizeof out));
 pFilename_dirent_offset[0]=LAUNCHER_FILE_DIRENT_INVALID;assert(!Launcher_ResolveIndexedLaunchName(0,out,sizeof out));pFilename_dirent_offset[0]=32;
 open_error=1;assert(!Launcher_ResolveIndexedLaunchName(0,out,sizeof out));open_error=0;
 read_error=1;assert(!Launcher_ResolveIndexedLaunchName(0,out,sizeof out));read_error=0;
 close_error=1;assert(!Launcher_ResolveIndexedLaunchName(0,out,sizeof out));
 puts("OMEGA production launch-name resolver: aliases/full names, 244-byte names, stale entries, capacity and I/O failures passed.");
}
'''
with tempfile.TemporaryDirectory() as d:
    c=Path(d)/'test.c'; exe=Path(d)/'test'
    c.write_text(pre+s[a:b]+post)
    subprocess.run([os.environ.get('CC','cc'),'-std=c99','-O1','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-pie','-no-pie',str(c),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
