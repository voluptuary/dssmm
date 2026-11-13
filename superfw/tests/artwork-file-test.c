/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <string.h>
#include "fatfs/ff.h"
#include "artwork_file.h"
static unsigned opens, closes, fail_read, fail_seek, fail_close;
static int gamecode_header, rom_file, filename_custom, filename_mirror, box_custom, gamecode_art;
static char last_path[288];
static uint8_t bmp[54+120*160];
_Alignas(2) static uint8_t pixels[ARTWORK_MAX_PIXELS];
static void put32(unsigned off, uint32_t value) {
  for(unsigned i=0;i<4;++i) bmp[off+i]=value>>(8*i);
}
FRESULT f_open(FIL *fp, const TCHAR *path, BYTE mode) {
  (void)mode; ++opens;
  strncpy(last_path,path,sizeof(last_path)-1); last_path[sizeof(last_path)-1]=0;
  rom_file=0;
  const char *dot=strrchr(path,'.');
  if(dot && (!strcmp(dot,".gba") || !strcmp(dot,".GBA"))) {
    rom_file=1; fp->obj.objsize=0xB0; fp->fptr=0; return FR_OK;
  }
  if(filename_custom && !strcmp(path,"/SYSTEM/IMGS/CUSTOM/Game Boy game (USA).bmp")) goto image;
  if(filename_mirror && !strcmp(path,"/SYSTEM/IMGS/Game Boy Advance/P/Game Boy game (USA).bmp")) goto image;
  if(box_custom && !strcmp(path,"/SYSTEM/IMGS2/CUSTOM/Game.bmp")) goto image;
  if(gamecode_art && !strcmp(path,"/SYSTEM/IMGS/B/P/BPRE.bmp")) goto image;
  return FR_NO_FILE;
image:
  fp->obj.objsize=sizeof(bmp); fp->fptr=0; return FR_OK;
}
FRESULT f_lseek(FIL *fp, FSIZE_t offset) {
  fp->fptr=offset; return fail_seek ? FR_DISK_ERR : FR_OK;
}
FRESULT f_read(FIL *fp, void *data, UINT size, UINT *got) {
  if(rom_file) {
    if(gamecode_header && fp->fptr==0xAC && size==4) memcpy(data,"BPRE",4);
    else if(fp->fptr==0xAC && size==4) memcpy(data,"????",4);
    else { assert(0 && "unexpected ROM header read"); }
    *got=fail_read ? size-1 : size; return FR_OK;
  }
  assert(fp->fptr+size<=sizeof(bmp));
  *got=fail_read ? size-1 : size;
  memcpy(data,bmp+fp->fptr,*got); return FR_OK;
}
FRESULT f_close(FIL *fp) { (void)fp; ++closes; return fail_close ? FR_DISK_ERR : FR_OK; }
int main(void) {
  char path[288]; artwork_info info;
  assert(artwork_path(path,sizeof(path),"/Game Boy/Game.gbc",0,0));
  assert(!strcmp(path,"/SYSTEM/IMGS/Game Boy/Game.bmp"));
  assert(artwork_path(path,sizeof(path),"/Game Boy/Game.gbc",1,1));
  assert(!strcmp(path,"/SYSTEM/IMGS2/CUSTOM/Game.bmp"));
  assert(artwork_path(path,sizeof(path),"/Game.gba",0,0));
  assert(!strcmp(path,"/SYSTEM/IMGS/Game.bmp"));
  const char *bad[]={"Game.gba","/../Game.gba","/A/./Game.gba","//Game.gba",
                     "/A\\Game.gba","/A:Game.gba","/Game.","/Game","/.gba","/"};
  for(unsigned i=0;i<sizeof(bad)/sizeof(bad[0]);++i) assert(!artwork_path(path,sizeof(path),bad[i],0,0));
  assert(!artwork_path(path,8,"/Game.gba",0,0));
  assert(!artwork_path(path,sizeof(path),"/Game.gba",2,0));
  memcpy(bmp,"BM",2); put32(2,sizeof(bmp)); put32(10,54); put32(14,40);
  put32(18,120); put32(22,(uint32_t)-80); bmp[26]=1; bmp[28]=16; put32(34,120*160);

  /* CUSTOM filename match wins, with mirrored filename as the next fallback. */
  filename_custom=1;
  assert(artwork_load("/Game Boy Advance/P/Game Boy game (USA).gba",0,pixels,sizeof(pixels),&info));
  assert(opens==1 && closes==1 && info.width==120);
  assert(!strcmp(last_path,"/SYSTEM/IMGS/CUSTOM/Game Boy game (USA).bmp"));
  filename_custom=0; filename_mirror=1; opens=closes=0;
  assert(artwork_load("/Game Boy Advance/P/Game Boy game (USA).gba",0,pixels,sizeof(pixels),&info));
  assert(opens==2 && closes==1 && info.width==120);
  assert(!strcmp(last_path,"/SYSTEM/IMGS/Game Boy Advance/P/Game Boy game (USA).bmp"));

  /* Box uses IMGS2 and supports a CUSTOM filename fallback. */
  filename_mirror=0; box_custom=1; opens=closes=0;
  put32(18,80); put32(22,(uint32_t)-80); put32(34,80*160);
  assert(artwork_load("/Game.gba",1,pixels,sizeof(pixels),&info));
  assert(opens==1 && closes==1 && info.width==80);
  assert(!strcmp(last_path,"/SYSTEM/IMGS2/CUSTOM/Game.bmp"));

  /* Original EZ-FLASH Title art uses the ROM game code at 0xAC. */
  box_custom=0; gamecode_header=1; gamecode_art=1; opens=closes=0;
  put32(18,120); put32(22,(uint32_t)-80); put32(34,120*160);
  assert(artwork_load("/Game Boy Advance/P/Pokemon FireRed.gba",0,pixels,sizeof(pixels),&info));
  assert(opens==4 && closes==2 && info.width==120);
  assert(!strcmp(last_path,"/SYSTEM/IMGS/B/P/BPRE.bmp"));
  gamecode_header=gamecode_art=0;

  opens=closes=0; fail_read=1;
  assert(!artwork_load("/Game.gba",0,pixels,sizeof(pixels),&info));
  fail_read=0; fail_seek=1;
  assert(!artwork_load("/Game.gba",0,pixels,sizeof(pixels),&info));
  fail_seek=0; fail_close=1;
  assert(!artwork_load("/Game.gba",0,pixels,sizeof(pixels),&info));
  fail_close=0; bmp[0]='X';
  assert(!artwork_load("/Game.gba",0,pixels,sizeof(pixels),&info));
  return 0;
}
