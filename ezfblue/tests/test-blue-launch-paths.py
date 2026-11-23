#!/usr/bin/env python3
"""Exercise production path resolution with FAT aliases and alias-free exFAT names."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1];source=(root/'targets/blue/source/ezbluekernel.c').read_text()
names=['static u32 Launcher_BuildFullPath(', 'static u32 Launcher_ResolveDirectoryEntry(', 'static const TCHAR *Launcher_PathIdentity(', 'static u32 Launcher_ResolveFullPath(']
parts=[]
for name in names:
 start=source.index(name)
 # Skip forward declarations; select the actual body.
 while source.find(';',start)<source.find('{',start):start=source.index(name,start+len(name))
 end=source.index('\n}',start)+2;parts.append(source[start:end])
with tempfile.TemporaryDirectory()as tmp:
 out=Path(tmp);(out/'paths.h').write_text('\n'.join(parts))
 code=r'''
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include "ff.h"
#include "launcher_path_policy.h"
typedef uint32_t u32;typedef uint8_t u8;
#define MAX_path_len 256
static DIR launcher_path_resolve_dir;static FILINFO launcher_path_resolve_info;
static const char *entry_name="Native launch test (USA).gba",*entry_alias="";
static const char *parent="/";static unsigned cursor;static int duplicate,read_error,close_error;
FRESULT f_opendir(DIR *d,const TCHAR *path){(void)d;parent=path;cursor=0;return FR_OK;}
FRESULT f_readdir(DIR *d,FILINFO *info){(void)d;memset(info,0,sizeof(*info));if(read_error)return FR_DISK_ERR;
 if(cursor++==0 || (duplicate&&cursor==2)){
  const char *name=!strcmp(parent,"/")?"Long directory name":entry_name;
  strcpy(info->fname,name);strcpy(info->altname,entry_alias);info->fsize=32768;
  if(!strcmp(parent,"/"))info->fattrib=AM_DIR;
  if(duplicate&&cursor==2)strcpy(info->fname,"Other.gba");
 }return FR_OK;}
FRESULT f_closedir(DIR *d){(void)d;return close_error?FR_DISK_ERR:FR_OK;}
#include "paths.h"
int main(void){char name[256],alias[13],path[256],full[512];u32 size;
 assert(Launcher_ResolveDirectoryEntry("/Long directory name",entry_name,name,sizeof(name),alias,sizeof(alias),&size,0));
 assert(!strcmp(name,entry_name)&&!alias[0]&&size==32768);
 assert(Launcher_ResolveFullPath("/Long directory name/Native launch test (USA).gba",512,path,sizeof(path),name,sizeof(name),alias,sizeof(alias),&size));
 assert(!strcmp(path,"/Long directory name")&&!strcmp(name,entry_name)&&!alias[0]);
 assert(Launcher_BuildFullPath(path,Launcher_PathIdentity(name,alias),full,sizeof(full)));
 assert(!strcmp(full,"/Long directory name/Native launch test (USA).gba"));
 entry_alias="NATIVE~1.GBA";
 assert(Launcher_ResolveDirectoryEntry("/Long directory name",entry_alias,name,sizeof(name),alias,sizeof(alias),&size,0));
 assert(!strcmp(alias,entry_alias)&&!strcmp(name,entry_name));
 duplicate=1;assert(!Launcher_ResolveDirectoryEntry("/Long directory name",entry_alias,name,sizeof(name),alias,sizeof(alias),&size,0));duplicate=0;
 read_error=1;assert(!Launcher_ResolveDirectoryEntry("/Long directory name",entry_name,name,sizeof(name),alias,sizeof(alias),&size,0));read_error=0;
 close_error=1;assert(!Launcher_ResolveDirectoryEntry("/Long directory name",entry_name,name,sizeof(name),alias,sizeof(alias),&size,0));close_error=0;
 entry_alias="";char longest[256];memset(longest,'A',251);strcpy(longest+251,".gba");entry_name=longest;
 assert(Launcher_ResolveDirectoryEntry("/Long directory name",entry_name,name,sizeof(name),alias,sizeof(alias),&size,0));
 assert(strlen(name)==255&&!alias[0]);
 assert(!Launcher_ResolveDirectoryEntry("/Long directory name",entry_name,name,255,alias,sizeof(alias),&size,0));
 puts("BLUE production launch paths: alias-free exFAT/LFN, FAT SFN, nested canonical paths, ambiguity, 255-byte names and read/close errors passed.");
}
'''
 (out/'test.c').write_text(code)
 subprocess.run(['gcc','-O1','-g','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-pie','-no-pie','-I',str(out),'-I',str(root/'targets/blue/source'),'-I',str(root/'targets/blue/source/ff15'),str(out/'test.c'),'-o',str(out/'test')],check=True)
 subprocess.run([str(out/'test')],check=True)
