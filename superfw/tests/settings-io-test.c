/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "posix-fatfs-fixture.h"
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
 char filename[1024];path(filename,SETTINGS_FILEPATH);FILE *binary=fopen(filename,"wb");assert(binary);
 assert(fwrite("Theme=Dark\0Colour=Blue\n",1,23,binary)==23);assert(!fclose(binary));
 assert(!settings_read_text(SETTINGS_FILEPATH,parse,&exists));
 assert(!settings_write_text("Theme = Light\n","",owned));
 for(unsigned i=0;i<8;i++)assert(!handles[i]);
 assert(f_unlink(SETTINGS_FILEPATH)==FR_OK);f_unlink("/SYSTEM/.sf-settings.tmp");
 char q[1024];path(q,"/SYSTEM");assert(!rmdir(q));assert(!rmdir(root));
 puts("Unified settings preserve unknown and sidecar values; repeat saves, short-write, rollback, recovery and oversize checks passed.");}
