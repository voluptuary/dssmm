/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <string.h>
#include "system_layout.h"
#include "config.h"
#include "nanoprintf.h"
#pragma GCC optimize ("Os")

/* FatFs rename does not move/overwrite existing destinations. Merge directories
 * only; conflicting files stop startup before pending SRAM can be replaced.
 * No data is copied through cartridge scratch RAM. */
static FRESULT merge_directory(const char *from, const char *to, unsigned depth) {
  if (depth > 8) return FR_INVALID_NAME;
  FILINFO info;
  FRESULT ret=f_stat(to,&info);
  if (ret==FR_NO_FILE || ret==FR_NO_PATH) return f_rename(from,to);
  if (ret!=FR_OK) return ret;
  if (!(info.fattrib&AM_DIR)) return FR_EXIST;
  DIR dir;
  ret=f_opendir(&dir,from);
  if(ret!=FR_OK)return ret;
  while ((ret=f_readdir(&dir,&info))==FR_OK && info.fname[0]) {
    if(!strcmp(info.fname,".") || !strcmp(info.fname,".."))continue;
    char source[MAX_FN_LEN],target[MAX_FN_LEN];
    int a=npf_snprintf(source,sizeof(source),"%s/%s",from,info.fname);
    int b=npf_snprintf(target,sizeof(target),"%s/%s",to,info.fname);
    if(a<0 || b<0 || (unsigned)a>=sizeof(source) || (unsigned)b>=sizeof(target)) {ret=FR_INVALID_NAME;break;}
    if(info.fattrib&AM_DIR) ret=merge_directory(source,target,depth+1);
    else ret=f_rename(source,target);
    if(ret!=FR_OK)break;
  }
  FRESULT close=f_closedir(&dir);
  if(ret!=FR_OK)return ret;
  if(close!=FR_OK)return close;
  return f_unlink(from); /* Only empty, successfully migrated directories. */
}

FRESULT system_layout_migrate(void) {
  FILINFO info;
  FRESULT ret=f_stat("/.superfw",&info);
  if(ret!=FR_NO_FILE && ret!=FR_NO_PATH) {
    if(ret!=FR_OK)return ret;
    if(!(info.fattrib&AM_DIR))return FR_INVALID_NAME;
    ret=f_mkdir(SYSTEM_DIR);
    if(ret!=FR_OK && ret!=FR_EXIST)return ret;
    ret=merge_directory("/.superfw",SYSTEM_DIR,0);
    if(ret!=FR_OK)return ret;
  }
  ret=f_stat("/SYSTEM/emulators",&info);
  if(ret==FR_NO_FILE || ret==FR_NO_PATH)return FR_OK;
  if(ret!=FR_OK)return ret;
  if(!(info.fattrib&AM_DIR))return FR_INVALID_NAME;
  return merge_directory("/SYSTEM/emulators","/SYSTEM/PLUG",0);
}
