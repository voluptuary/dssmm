/* SPDX-License-Identifier: GPL-3.0-or-later */
#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "fatfs/ff.h"
#include "settings_io.h"
#include "config.h"
static char root[128];static FILE *handles[8];static int short_write,fail_commit;
static void path(char out[1024],const char *p){assert(snprintf(out,1024,"%s%s",root,p)<1024);}
static FRESULT error(void){return errno==ENOENT?FR_NO_FILE:FR_DISK_ERR;}
FRESULT f_open(FIL *f,const TCHAR *p,BYTE flags){char q[1024];path(q,p);FILE *h=fopen(q,flags&FA_WRITE?"wb":"rb");if(!h)return error();for(unsigned i=0;i<8;i++)if(!handles[i]){handles[i]=h;memset(f,0,sizeof(*f));f->obj.id=i;fseek(h,0,SEEK_END);f->obj.objsize=ftell(h);rewind(h);return FR_OK;}abort();}
FRESULT f_close(FIL *f){FILE *h=handles[f->obj.id];handles[f->obj.id]=0;return fclose(h)?FR_DISK_ERR:FR_OK;}
FRESULT f_read(FIL *f,void *buf,UINT n,UINT *got){FILE *h=handles[f->obj.id];*got=fread(buf,1,n,h);return ferror(h)?FR_DISK_ERR:FR_OK;}
FRESULT f_write(FIL *f,const void *buf,UINT n,UINT *got){*got=fwrite(buf,1,n-(short_write&&n?1:0),handles[f->obj.id]);return FR_OK;}
FRESULT f_stat(const TCHAR *p,FILINFO *i){(void)i;char q[1024];struct stat st;path(q,p);return stat(q,&st)?error():FR_OK;}
FRESULT f_mkdir(const TCHAR *p){char q[1024];path(q,p);return mkdir(q,0700)?(errno==EEXIST?FR_EXIST:error()):FR_OK;}
FRESULT f_unlink(const TCHAR *p){char q[1024];path(q,p);return unlink(q)?error():FR_OK;}
FRESULT f_rename(const TCHAR *a,const TCHAR *b){char x[1024],y[1024];path(x,a);path(y,b);if(fail_commit&&!strcmp(a,"/SYSTEM/.sf-settings.tmp"))return FR_DISK_ERR;struct stat st;if(!stat(y,&st))return FR_EXIST;return rename(x,y)?error():FR_OK;}
static void write_file(const char *p,const char *s){char q[1024];path(q,p);FILE *f=fopen(q,"wb");assert(f);assert(fwrite(s,1,strlen(s),f)==strlen(s));assert(!fclose(f));}
static unsigned read_file(const char *p,char *out,unsigned size){char q[1024];path(q,p);FILE *f=fopen(q,"rb");assert(f);unsigned n=fread(out,1,size-1,f);out[n]=0;assert(!fclose(f));return n;}
static bool owned(const char *key){return !strcmp(key,"Theme")||!strcmp(key,"font")||!strcmp(key,"SuperFW.font");}
static unsigned seen;
static void parse(const char *key,const char *value){if(!strcmp(key,"Theme")){assert(!strcmp(value,"Dark"));seen++;}}
int main(void){char tmp[]="/tmp/dssmm-settings-XXXXXX";assert(mkdtemp(tmp));strcpy(root,tmp);assert(f_mkdir("/SYSTEM")==FR_OK);
 write_file(SETTINGS_FILEPATH,"\xef\xbb\xbfTheme = Dark\r\n# keep my notes\nUnknown DE = keep\nSuperFW.future=retain\n");
 bool exists=false;assert(settings_read_text(SETTINGS_FILEPATH,parse,&exists)&&exists&&seen==1);
 write_file(UISETTINGS_FILEPATH,"font=1\n# legacy notes\nLegacyFuture=yes\n");
 assert(settings_write_text("Theme = Light\n","SuperFW.font=1\n",owned));
 char text[10000],before[10000];unsigned n=read_file(SETTINGS_FILEPATH,text,sizeof(text));
 assert(!strncmp(text,"# DSSMM shared settings\nTheme = Light\n",37));
 assert(!strstr(text,"Dark"));
 assert(strstr(text,"# keep my notes")&&strstr(text,"Unknown DE = keep")&&strstr(text,"LegacyFuture=yes")&&strstr(text,"# legacy notes"));
 assert(strstr(text,"SuperFW.future=retain")&&strstr(text,"SuperFW.font=1"));
 assert(f_stat(UISETTINGS_FILEPATH,0)==FR_NO_FILE);
 strcpy(before,text);assert(settings_write_text("Theme = Light\n","SuperFW.font=1\n",owned));
 assert(read_file(SETTINGS_FILEPATH,text,sizeof(text))==n&&!strcmp(text,before));
 short_write=1;assert(!settings_write_text("Theme = Dark\n","SuperFW.font=0\n",owned));short_write=0;
 read_file(SETTINGS_FILEPATH,text,sizeof(text));assert(!strcmp(text,before));
 fail_commit=1;assert(!settings_write_text("Theme = Dark\n","SuperFW.font=0\n",owned));fail_commit=0;
 read_file(SETTINGS_FILEPATH,text,sizeof(text));assert(!strcmp(text,before));
 assert(f_rename(SETTINGS_FILEPATH,"/SYSTEM/.sf-settings.bak")==FR_OK);
 assert(settings_write_text("Theme = Light\n","SuperFW.font=1\n",owned));
 read_file(SETTINGS_FILEPATH,text,sizeof(text));assert(!strcmp(text,before));
 memset(text,'x',9000);text[9000]=0;write_file(SETTINGS_FILEPATH,text);
 assert(!settings_write_text("Theme = Light\n","",owned));assert(read_file(SETTINGS_FILEPATH,before,sizeof(before))==9000&&!strcmp(text,before));
 for(unsigned i=0;i<8;i++)assert(!handles[i]);
 assert(f_unlink(SETTINGS_FILEPATH)==FR_OK);f_unlink("/SYSTEM/.sf-settings.tmp");
 char q[1024];path(q,"/SYSTEM");assert(!rmdir(q));assert(!rmdir(root));
 puts("Unified settings preserve unknown and sidecar values; repeat saves, short-write, rollback, recovery and oversize checks passed.");}
