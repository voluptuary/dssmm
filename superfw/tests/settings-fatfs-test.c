/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "settings.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"
volatile unsigned frame_count;
unsigned lang_lookup(uint16_t code){(void)code;return 0;}
uint16_t lang_getcode(void){return 0x6e65;}
static uint8_t *disk;
static unsigned sectors;
static unsigned fail_next_write;
static FATFS filesystem;
DSTATUS disk_status(BYTE drive){(void)drive;return 0;}
DSTATUS disk_initialize(BYTE drive){(void)drive;return 0;}
DRESULT disk_read(BYTE drive,BYTE *buffer,LBA_t sector,UINT count){(void)drive;if(sector>sectors||count>sectors-sector)return RES_PARERR;memcpy(buffer,disk+sector*512,count*512);return RES_OK;}
DRESULT disk_write(BYTE drive,const BYTE *buffer,LBA_t sector,UINT count){(void)drive;if(sector>sectors||count>sectors-sector)return RES_PARERR;if(fail_next_write){fail_next_write=0;return RES_ERROR;}memcpy(disk+sector*512,buffer,count*512);return RES_OK;}
DRESULT disk_ioctl(BYTE drive,BYTE command,void *buffer){(void)drive;(void)command;(void)buffer;return RES_OK;}
static void write16(unsigned offset,unsigned value){disk[offset]=value;disk[offset+1]=value>>8;}
static void write32(unsigned offset,uint32_t value){write16(offset,value);write16(offset+2,value>>16);}
static void format_disk(bool fat32) {
 sectors=fat32?131072:8192;disk=calloc(sectors,512);assert(disk);
 disk[0]=0xeb;disk[1]=0x3c;disk[2]=0x90;memcpy(disk+3,"DSSMM   ",8);
 write16(11,512);disk[13]=1;disk[16]=2;disk[21]=0xf8;write16(510,0xaa55);
 if(fat32) {
  write16(14,32);write32(32,sectors);write32(36,1024);write32(44,2);
  write16(48,1);write16(50,6);disk[66]=0x29;memcpy(disk+82,"FAT32   ",8);
  write32(512,0x41615252);write32(512+484,0x61417272);
  write32(512+488,UINT32_MAX);write32(512+492,UINT32_MAX);write32(512+508,0xaa550000);
  memcpy(disk+6*512,disk,512);
  for(unsigned copy=0;copy<2;++copy){unsigned offset=(32+copy*1024)*512;write32(offset,0x0ffffff8);write32(offset+4,0x0fffffff);write32(offset+8,0x0fffffff);}
 } else {
  write16(14,1);write16(17,512);write16(19,sectors);write16(22,32);
  disk[38]=0x29;memcpy(disk+54,"FAT16   ",8);
  for(unsigned copy=0;copy<2;++copy){unsigned offset=(1+copy*32)*512;write16(offset,0xfff8);write16(offset+2,0xffff);}
 }
}
static void remount(void) {
 assert(f_mount(0,"",0)==FR_OK);memset(&filesystem,0,sizeof(filesystem));assert(f_mount(&filesystem,"",1)==FR_OK);
 menu_theme=0;menu_accent=0;browser_view=0;artwork_mode=0;load_settings();
}
int main(void) {
 for(unsigned fat32=0;fat32<2;++fat32) {
  format_disk(fat32);assert(f_mount(&filesystem,"",1)==FR_OK);assert(f_mkdir("/SYSTEM")==FR_OK);
  for(unsigned theme=6;theme<8;++theme)for(unsigned accent=0;accent<5;++accent)
  for(unsigned view=0;view<3;++view)for(unsigned art=0;art<3;++art) {
   menu_theme=theme;menu_accent=accent;browser_view=view;artwork_mode=art;
   assert(save_settings());remount();
   assert(menu_theme==theme&&menu_accent==accent&&browser_view==view&&artwork_mode==art);
  }
  menu_theme=6;menu_accent=2;browser_view=1;artwork_mode=1;
  assert(save_settings());
  menu_theme=7;fail_next_write=1;
  assert(!save_settings());
  remount();
  assert(menu_theme==6&&menu_accent==2&&browser_view==1&&artwork_mode==1);
  assert(f_mount(0,"",0)==FR_OK);free(disk);
 }
 puts("Actual FatFs FAT16/FAT32 settings save/unmount/remount and failed-write recovery passed (RAM disk, not card hardware).");
}
