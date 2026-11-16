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
/* Deliberately ignored legacy files used only by tests. */
#define UISETTINGS_FILEPATH "/SYSTEM/UI-SETTINGS.TXT"
static char root[128];static FILE *handles[8];static int short_write,fail_commit,corrupt_commit,read_error,short_read,read_error_after,close_error;
static void path(char out[1024],const char *p){assert(snprintf(out,1024,"%s%s",root,p)<1024);}
static FRESULT error(void){return errno==ENOENT?FR_NO_FILE:FR_DISK_ERR;}
FRESULT f_open(FIL *f,const TCHAR *p,BYTE flags){char q[1024];path(q,p);FILE *h=fopen(q,flags&FA_WRITE?"wb":"rb");if(!h)return error();for(unsigned i=0;i<8;i++)if(!handles[i]){handles[i]=h;memset(f,0,sizeof(*f));f->obj.id=i;fseek(h,0,SEEK_END);f->obj.objsize=ftell(h);rewind(h);return FR_OK;}abort();}
FRESULT f_close(FIL *f){FILE *h=handles[f->obj.id];handles[f->obj.id]=0;return fclose(h)||close_error?FR_DISK_ERR:FR_OK;}
FRESULT f_read(FIL *f,void *buf,UINT n,UINT *got){FILE *h=handles[f->obj.id];if(read_error||(read_error_after&&ftell(h)>=read_error_after)){*got=0;return FR_DISK_ERR;}if(short_read&&n)--n;*got=fread(buf,1,n,h);return ferror(h)?FR_DISK_ERR:FR_OK;}
FRESULT f_write(FIL *f,const void *buf,UINT n,UINT *got){*got=fwrite(buf,1,n-(short_write&&n?1:0),handles[f->obj.id]);return FR_OK;}
FRESULT f_stat(const TCHAR *p,FILINFO *i){(void)i;char q[1024];struct stat st;path(q,p);return stat(q,&st)?error():FR_OK;}
FRESULT f_mkdir(const TCHAR *p){char q[1024];path(q,p);return mkdir(q,0700)?(errno==EEXIST?FR_EXIST:error()):FR_OK;}
FRESULT f_unlink(const TCHAR *p){char q[1024];path(q,p);return unlink(q)?error():FR_OK;}
FRESULT f_rename(const TCHAR *a,const TCHAR *b){char x[1024],y[1024];path(x,a);path(y,b);if(fail_commit&&!strcmp(a,"/SYSTEM/.sf-settings.tmp"))return FR_DISK_ERR;struct stat st;if(!stat(y,&st))return FR_EXIST;if(rename(x,y))return error();if(corrupt_commit&&!strcmp(a,"/SYSTEM/.sf-settings.tmp")){FILE *h=fopen(y,"r+b");assert(h);fputc('!',h);assert(!fclose(h));}return FR_OK;}
static void write_file(const char *p,const char *s){char q[1024];path(q,p);FILE *f=fopen(q,"wb");assert(f);assert(fwrite(s,1,strlen(s),f)==strlen(s));assert(!fclose(f));}
static unsigned read_file(const char *p,char *out,unsigned size){char q[1024];path(q,p);FILE *f=fopen(q,"rb");assert(f);unsigned n=fread(out,1,size-1,f);out[n]=0;assert(!fclose(f));return n;}
