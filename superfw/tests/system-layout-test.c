#define _GNU_SOURCE
#define DIR POSIX_DIR
#include <dirent.h>
#undef DIR
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "system_layout.h"
static char root[128];
static unsigned rename_count,fail_at;
static struct {DIR *key;POSIX_DIR *dir;} handles[16];
static void path(char *out,const char *p){assert(snprintf(out,1024,"%s%s",root,p)<1024);}
static FRESULT err(void){return errno==ENOENT?FR_NO_FILE:FR_DISK_ERR;}
FRESULT f_stat(const TCHAR *p,FILINFO *info){char q[1024];path(q,p);struct stat s;if(stat(q,&s))return err();if(info){memset(info,0,sizeof(*info));info->fattrib=S_ISDIR(s.st_mode)?AM_DIR:0;}return FR_OK;}
FRESULT f_mkdir(const TCHAR *p){char q[1024];path(q,p);if(!mkdir(q,0700))return FR_OK;return errno==EEXIST?FR_EXIST:err();}
FRESULT f_rename(const TCHAR *a,const TCHAR *b){char x[1024],y[1024];path(x,a);path(y,b);if(++rename_count==fail_at)return FR_DISK_ERR;struct stat s;if(!stat(y,&s))return FR_EXIST;return rename(x,y)?err():FR_OK;}
FRESULT f_unlink(const TCHAR *p){char q[1024];path(q,p);return rmdir(q)?err():FR_OK;}
FRESULT f_opendir(DIR *d,const TCHAR *p){char q[1024];path(q,p);for(unsigned i=0;i<16;i++)if(!handles[i].key){handles[i].dir=opendir(q);if(!handles[i].dir)return err();handles[i].key=d;return FR_OK;}return FR_TOO_MANY_OPEN_FILES;}
FRESULT f_readdir(DIR *d,FILINFO *info){for(unsigned i=0;i<16;i++)if(handles[i].key==d){struct dirent *e=readdir(handles[i].dir);memset(info,0,sizeof(*info));if(e){assert(strlen(e->d_name)<sizeof(info->fname));strcpy(info->fname,e->d_name);info->fattrib=e->d_type==DT_DIR?AM_DIR:0;}return FR_OK;}return FR_INVALID_OBJECT;}
FRESULT f_closedir(DIR *d){for(unsigned i=0;i<16;i++)if(handles[i].key==d){assert(!closedir(handles[i].dir));handles[i].key=0;return FR_OK;}return FR_INVALID_OBJECT;}
static void file(const char *p,const char *data){char q[1024];path(q,p);FILE *f=fopen(q,"wb");assert(f);assert(fwrite(data,1,strlen(data),f)==strlen(data));assert(!fclose(f));}
static void expect(const char *p,const char *data){char q[1024],buf[128]={0};path(q,p);FILE *f=fopen(q,"rb");assert(f);assert(fread(buf,1,sizeof(buf)-1,f)==strlen(data));assert(!strcmp(buf,data));fclose(f);}
int main(void){char tmp[]="/tmp/dssmm-system-layout-XXXXXX";assert(mkdtemp(tmp));strcpy(root,tmp);
 assert(system_layout_migrate()==FR_OK);
 assert(f_mkdir("/.superfw")==FR_OK);assert(f_mkdir("/.superfw/config")==FR_OK);
 file("/.superfw/settings.txt","preferences");file("/.superfw/pending-save.txt","pending SRAM");file("/.superfw/config/Game.gba.txt","game config");
 assert(f_mkdir("/SYSTEM")==FR_OK);assert(f_mkdir("/SYSTEM/IMGS")==FR_OK);file("/SYSTEM/IMGS/keep.bmp","artwork");
 assert(system_layout_migrate()==FR_OK);expect("/SYSTEM/settings.txt","preferences");expect("/SYSTEM/pending-save.txt","pending SRAM");expect("/SYSTEM/config/Game.gba.txt","game config");expect("/SYSTEM/IMGS/keep.bmp","artwork");assert(f_stat("/.superfw",0)==FR_NO_FILE);
 assert(system_layout_migrate()==FR_OK);
 /* Destination conflict must not overwrite either file or consume pending SRAM. */
 assert(f_mkdir("/.superfw")==FR_OK);file("/.superfw/pending-save.txt","different pending");
 assert(system_layout_migrate()==FR_EXIST);expect("/.superfw/pending-save.txt","different pending");expect("/SYSTEM/pending-save.txt","pending SRAM");
 char q[1024];path(q,"/.superfw/pending-save.txt");assert(!unlink(q));
 /* Failed move can be resumed; existing merged subtrees are preserved. */
 assert(f_mkdir("/.superfw/config")==FR_OK);file("/.superfw/config/Other.txt","other");fail_at=rename_count+1;
 assert(system_layout_migrate()==FR_DISK_ERR);expect("/.superfw/config/Other.txt","other");fail_at=0;
 assert(system_layout_migrate()==FR_OK);expect("/SYSTEM/config/Other.txt","other");expect("/SYSTEM/config/Game.gba.txt","game config");
 for(unsigned i=0;i<16;i++)assert(!handles[i].key);
 /* Remove only the fixture's known files and directories. */
 const char *files[]={"/SYSTEM/settings.txt","/SYSTEM/pending-save.txt","/SYSTEM/config/Game.gba.txt","/SYSTEM/config/Other.txt","/SYSTEM/IMGS/keep.bmp"};for(unsigned i=0;i<5;i++){path(q,files[i]);assert(!unlink(q));}
 assert(f_unlink("/SYSTEM/config")==FR_OK);assert(f_unlink("/SYSTEM/IMGS")==FR_OK);assert(f_unlink("/SYSTEM")==FR_OK);assert(!rmdir(root));
 puts("SYSTEM migration preserves resources, artwork, conflicts, pending SRAM and retry state.");}
