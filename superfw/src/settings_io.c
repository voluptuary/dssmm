/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <string.h>
#include <strings.h>
#include "settings_io.h"
#include "fatfs/ff.h"
#include "config.h"
#pragma GCC optimize ("Os")
#define CONFIG_CAPACITY 8192
#define CONFIG_TEMP "/SYSTEM/.sf-settings.tmp"
#define CONFIG_BACKUP "/SYSTEM/.sf-settings.bak"
/* GBA RAM: ordinary byte parsing must never use cartridge SDRAM. */
static char original[CONFIG_CAPACITY] __attribute__((section(".sbss"),aligned(__alignof__(FIL))));
static unsigned original_size;
static bool read_original(const char *path,bool *exists) {
  FIL file;*exists=false;original_size=0;original[0]=0;
  FRESULT result=f_open(&file,path,FA_READ);
  if(result==FR_NO_FILE || result==FR_NO_PATH)return true;
  if(result!=FR_OK)return false;
  *exists=true;UINT size=0,extra=0;char end;
  result=f_read(&file,original,sizeof(original)-1,&size);
  if(result==FR_OK)result=f_read(&file,&end,1,&extra);
  FRESULT closed=f_close(&file);
  if(result!=FR_OK || closed!=FR_OK || extra || memchr(original,0,size))return false;
  if(size>=3 && !memcmp(original,"\xef\xbb\xbf",3)) {
    memmove(original,original+3,size-3);size-=3;
  }
  original_size=size;original[size]=0;return true;
}
static char *trim(char *text) {
  while(*text && (unsigned char)*text<=32)++text;
  char *end=text+strlen(text);while(end>text && (unsigned char)end[-1]<=32)*--end=0;
  return text;
}
static bool split(char *line,char **key,char **value) {
  *key=trim(line);if(!**key || **key=='#' || **key==';')return false;
  char *eq=strchr(*key,'=');if(!eq)return false;
  *eq=0;*key=trim(*key);*value=trim(eq+1);return **key!=0;
}
bool settings_read_text(const char *path,settings_line_fn callback,bool *exists) {
  if(settings_write_pending())return false;
  /* Interrupted replacement: restore the original if commit never completed. */
  if(!strcmp(path,SETTINGS_FILEPATH) && f_stat(path,0)==FR_NO_FILE && f_stat(CONFIG_BACKUP,0)==FR_OK)
    if(f_rename(CONFIG_BACKUP,path)!=FR_OK)return false;
  if(!read_original(path,exists))return false;
  char *line=original;
  if(original_size>=3 && !memcmp(line,"\xef\xbb\xbf",3))line+=3;
  while(*line) {
    char *end=strchr(line,'\n');if(end)*end=0;
    char *key,*value;if(split(line,&key,&value))callback(key,value);
    if(!end)break;
    line=end+1;
  }
  return true;
}
static uint32_t output_hash, output_size;
static char *staging;
static uint32_t text_hash(uint32_t hash, const void *data, unsigned size) {
  const unsigned char *p = data;
  while (size--) hash = (hash ^ *p++) * 16777619u;
  return hash;
}
static bool append_bytes(const void *data,unsigned size) {
  if(size>=CONFIG_CAPACITY-output_size)return false;
  memcpy(staging+output_size,data,size);
  output_hash=text_hash(output_hash,data,size);output_size+=size;
  return true;
}
static bool preserve(settings_owned_fn owned,bool private_lines) {
  unsigned position=0;
  while(position<original_size) {
    unsigned end=position;while(end<original_size && original[end]!='\n')++end;
    unsigned count=end-position+(end<original_size);
    char prefix[160];unsigned n=end-position;if(n>=sizeof(prefix))n=sizeof(prefix)-1;
    memcpy(prefix,original+position,n);prefix[n]=0;
    char *key=0,*value=0;bool entry=split(prefix,&key,&value);
    bool native=entry && !strncasecmp(key,"SuperFW.",8);
    bool marker=!strncmp(original+position,"# DSSMM shared settings",strlen("# DSSMM shared settings")) || !strncmp(original+position,"# SuperFW-specific settings",strlen("# SuperFW-specific settings"));
    if(!marker && native==private_lines && !(entry && owned(key)))
      if(!append_bytes(original+position,count) || (end==original_size && !append_bytes("\n",1)))return false;
    position=end+1;
  }
  return true;
}
/* Bounded filesystem steps; the existing text buffer owns the snapshot.
 * FIL occupies unused tail bytes. Very large configs use a synchronous stack FIL. */
enum {Idle,Open,Write,Close,Backup,MoveOld,Commit,VerifyOpen,VerifyRead,VerifyClose,Cleanup};
static unsigned phase,position;
static bool existed,moved,committed,opened;
static FIL *transaction;
static uint32_t verify_hash,verify_size;
bool settings_write_pending(void) {return phase!=Idle;}
static int write_failed(void) {
  if(opened)f_close(transaction);
  opened=false;
  if(committed) {
    if(f_unlink(SETTINGS_FILEPATH)==FR_OK && moved)f_rename(CONFIG_BACKUP,SETTINGS_FILEPATH);
  } else if(moved)f_rename(CONFIG_BACKUP,SETTINGS_FILEPATH);
  f_unlink(CONFIG_TEMP);phase=Idle;return -1;
}
int settings_write_step(void) {
  UINT count;
  switch(phase) {
    case Idle:return 1;
    case Open:
      if(f_open(transaction,CONFIG_TEMP,FA_WRITE|FA_CREATE_ALWAYS)!=FR_OK)return write_failed();
      opened=true;phase=Write;break;
    case Write: {
      unsigned size=output_size-position;if(size>512)size=512;
      if(f_write(transaction,original+position,size,&count)!=FR_OK || count!=size)return write_failed();
      position+=size;if(position==output_size)phase=Close;break;
    }
    case Close:
      opened=false;if(f_close(transaction)!=FR_OK)return write_failed();phase=Backup;break;
    case Backup:
      if(f_stat(CONFIG_BACKUP,0)==FR_OK && f_unlink(CONFIG_BACKUP)!=FR_OK)return write_failed();
      phase=MoveOld;break;
    case MoveOld:
      if(existed) {
        if(f_rename(SETTINGS_FILEPATH,CONFIG_BACKUP)!=FR_OK)return write_failed();
        moved=true;
      }
      phase=Commit;break;
    case Commit:
      if(f_rename(CONFIG_TEMP,SETTINGS_FILEPATH)!=FR_OK)return write_failed();
      committed=true;phase=VerifyOpen;break;
    case VerifyOpen:
      if(f_open(transaction,SETTINGS_FILEPATH,FA_READ)!=FR_OK)return write_failed();
      opened=true;verify_hash=2166136261u;verify_size=0;phase=VerifyRead;break;
    case VerifyRead: {
      char buffer[256];
      if(f_read(transaction,buffer,sizeof(buffer),&count)!=FR_OK)return write_failed();
      verify_hash=text_hash(verify_hash,buffer,count);verify_size+=count;
      if(!count)phase=VerifyClose;
      break;
    }
    case VerifyClose:
      opened=false;
      if(f_close(transaction)!=FR_OK || verify_size!=output_size || verify_hash!=output_hash)return write_failed();
      phase=Cleanup;break;
    case Cleanup:
      if(moved)f_unlink(CONFIG_BACKUP);
      phase=Idle;return 1;
  }
  return 0;
}
bool settings_write_start(const char *shared,const char *private_text,settings_owned_fn owned) {
  if(phase)return false;
  bool exists;
  if(f_stat(SETTINGS_FILEPATH,0)==FR_NO_FILE && f_stat(CONFIG_BACKUP,0)==FR_OK)
    if(f_rename(CONFIG_BACKUP,SETTINGS_FILEPATH)!=FR_OK)return false;
  if(!read_original(SETTINGS_FILEPATH,&exists))return false;
  /* Temporary output is on the user stack only during snapshot construction. */
  char output[CONFIG_CAPACITY];staging=output;
  output_hash=2166136261u;output_size=0;
  bool ok=append_bytes("# DSSMM shared settings\n",strlen("# DSSMM shared settings\n")) && append_bytes(shared,strlen(shared)) &&
      preserve(owned,false) && append_bytes("# SuperFW-specific settings\n",strlen("# SuperFW-specific settings\n")) &&
      append_bytes(private_text,strlen(private_text)) && preserve(owned,true);
  staging=0;if(!ok)return false;
  memcpy(original,output,output_size);
  existed=exists;moved=committed=opened=false;position=0;phase=Open;
  f_mkdir(SYSTEM_DIR);
  unsigned align=__alignof__(FIL);
  unsigned aligned=(output_size+align-1)&~(align-1);
  if(aligned+sizeof(FIL)<=sizeof(original))transaction=(FIL*)(original+aligned);
  else {
    /* Preserve support for near-capacity files without adding a persistent buffer. */
    FIL file;transaction=&file;
    int result;do{result=settings_write_step();}while(!result);
    return result>0;
  }
  return true;
}
bool settings_write_text(const char *shared,const char *private_text,settings_owned_fn owned) {
  if(!settings_write_start(shared,private_text,owned))return false;
  int result;do{result=settings_write_step();}while(!result);
  return result>0;
}
