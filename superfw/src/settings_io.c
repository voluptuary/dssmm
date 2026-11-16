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
static char original[CONFIG_CAPACITY] __attribute__((section(".sbss"),aligned(4)));
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
static bool write_bytes(FIL *file,const void *data,unsigned size) {
  UINT written=0;return f_write(file,data,size,&written)==FR_OK && written==size;
}
static bool preserve(FIL *out,settings_owned_fn owned,bool private_lines) {
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
      if(!write_bytes(out,original+position,count) || (end==original_size && !write_bytes(out,"\n",1)))return false;
    position=end+1;
  }
  return true;
}
bool settings_write_text(const char *shared,const char *private_text,settings_owned_fn owned) {
  bool exists;
  if(f_stat(SETTINGS_FILEPATH,0)==FR_NO_FILE && f_stat(CONFIG_BACKUP,0)==FR_OK)
    if(f_rename(CONFIG_BACKUP,SETTINGS_FILEPATH)!=FR_OK)return false;
  if(!read_original(SETTINGS_FILEPATH,&exists))return false; /* Refuse oversize/error; never truncate. */
  /* Preserve unknown legacy UI entries/comments before retiring the sidecar. */
  FIL legacy;
  FRESULT legacy_result=f_open(&legacy,UISETTINGS_FILEPATH,FA_READ);
  if(legacy_result==FR_OK) {
    unsigned separator=original_size && original[original_size-1]!='\n';
    if(original_size+separator>=sizeof(original)){f_close(&legacy);return false;}
    unsigned available=sizeof(original)-1-original_size-separator;
    if(f_size(&legacy)>available){f_close(&legacy);return false;}
    if(separator)original[original_size++]='\n';
    UINT count=0;FSIZE_t expected=f_size(&legacy);
    legacy_result=f_read(&legacy,original+original_size,available,&count);
    FRESULT close=f_close(&legacy);
    if(legacy_result!=FR_OK || close!=FR_OK || count!=expected)return false;
    original_size+=count;original[original_size]=0;
  }else if(legacy_result!=FR_NO_FILE && legacy_result!=FR_NO_PATH)return false;
  f_mkdir(SYSTEM_DIR);
  FIL out;if(f_open(&out,CONFIG_TEMP,FA_WRITE|FA_CREATE_ALWAYS)!=FR_OK)return false;
  bool ok=write_bytes(&out,"# DSSMM shared settings\n",strlen("# DSSMM shared settings\n")) && write_bytes(&out,shared,strlen(shared)) &&
      preserve(&out,owned,false) && write_bytes(&out,"# SuperFW-specific settings\n",strlen("# SuperFW-specific settings\n")) &&
      write_bytes(&out,private_text,strlen(private_text)) && preserve(&out,owned,true);
  if(f_close(&out)!=FR_OK)ok=false;
  if(!ok){f_unlink(CONFIG_TEMP);return false;}
  if(f_stat(CONFIG_BACKUP,0)==FR_OK && f_unlink(CONFIG_BACKUP)!=FR_OK)return false;
  if(exists && f_rename(SETTINGS_FILEPATH,CONFIG_BACKUP)!=FR_OK)return false;
  if(f_rename(CONFIG_TEMP,SETTINGS_FILEPATH)!=FR_OK) {
    if(exists)f_rename(CONFIG_BACKUP,SETTINGS_FILEPATH);
    return false;
  }
  if(exists)f_unlink(CONFIG_BACKUP);
  f_unlink(UISETTINGS_FILEPATH); /* Retire the legacy sidecar only after success. */
  return true;
}
