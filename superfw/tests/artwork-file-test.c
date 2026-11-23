/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <string.h>
#include "fatfs/ff.h"
#include "artwork_file.h"
static unsigned opens, closes, seeks, fail_read, fail_seek, fail_close;
static unsigned image_reads, fail_tail, rom_opens;
static bool folder_title,folder_box;
static int gamecode_header, rom_file, filename_custom, filename_mirror, box_custom, gamecode_art, gamecode_flat_art;
static char last_path[288], last_rom_path[288];
static char rom_code[5]="BPRE";
static uint8_t bmp[54+120*160];
static unsigned image_size=sizeof(bmp);
#ifdef TEST_CART_BUS
static uint8_t *bmp_buffer=(void *)0x08000000;
#define BMP_BUFFER_SIZE ARTWORK_BMP_MAX_SIZE
#else
_Alignas(4) static uint8_t bmp_buffer[ARTWORK_BMP_MAX_SIZE];
#define BMP_BUFFER_SIZE sizeof(bmp_buffer)
#endif
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
    ++rom_opens;strcpy(last_rom_path,path);
    rom_file=1; fp->obj.objsize=0xB0; fp->fptr=0; return FR_OK;
  }
  if(folder_title && !strcmp(path,"/SYSTEM/IMGS/CUSTOM/Game Boy Advance.bmp")) goto image;
  if(folder_box && !strcmp(path,"/SYSTEM/IMGS2/CUSTOM/Game Boy Advance.bmp")) goto image;
  if(filename_custom && !strcmp(path,"/SYSTEM/IMGS/CUSTOM/Game Boy game (USA).bmp")) goto image;
  if(filename_mirror && !strcmp(path,"/SYSTEM/IMGS/Game Boy Advance/P/Game Boy game (USA).bmp")) goto image;
  if(box_custom && !strcmp(path,"/SYSTEM/IMGS2/CUSTOM/Game.bmp")) goto image;
  if(gamecode_art && (!strcmp(path,"/SYSTEM/IMGS/B/P/BPRE.bmp") || !strcmp(path,"/SYSTEM/IMGS/T/C/TCHK.bmp"))) goto image;
  if(gamecode_flat_art && (!strcmp(path,"/SYSTEM/IMGS/BPRE.bmp") || !strcmp(path,"/SYSTEM/IMGS/TCHK.bmp"))) goto image;
  return FR_NO_FILE;
image:
  fp->obj.objsize=image_size; fp->fptr=0; return FR_OK;
}
FRESULT f_lseek(FIL *fp, FSIZE_t offset) {
  ++seeks; fp->fptr=offset; return fail_seek ? FR_DISK_ERR : FR_OK;
}
FRESULT f_read(FIL *fp, void *data, UINT size, UINT *got) {
  if(rom_file) {
    if(gamecode_header && fp->fptr==0xAC && size==4) memcpy(data,rom_code,4);
    else if(fp->fptr==0xAC && size==4) memcpy(data,"????",4);
    else { assert(0 && "unexpected ROM header read"); }
    *got=fail_read ? size-1 : size; fp->fptr += *got; return FR_OK;
  }
  #ifndef TEST_CART_BUS
  if(data==bmp_buffer) assert(!((uintptr_t)data&3) && !(size&3));
  #endif
  ++image_reads;
  assert(fp->fptr+size<=sizeof(bmp));
  *got=fail_read ? (data==bmp_buffer ? (size-1)&~3u : size-1) : fail_tail && size<4 ? size-1 : size;
  memcpy(data,bmp+fp->fptr,*got); fp->fptr += *got; return FR_OK;
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
  assert(artwork_load("/Game Boy Advance/P/Game Boy game (USA).gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(opens==1 && closes==1 && seeks==0 && image_reads==2 && info.width==120);
  assert(!strcmp(last_path,"/SYSTEM/IMGS/CUSTOM/Game Boy game (USA).bmp"));
  filename_custom=0; filename_mirror=1; opens=closes=seeks=0;
  assert(artwork_load("/Game Boy Advance/P/Game Boy game (USA).gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(opens==2 && closes==1 && seeks==0 && image_reads==4 && info.width==120);
  assert(!strcmp(last_path,"/SYSTEM/IMGS/Game Boy Advance/P/Game Boy game (USA).bmp"));

  /* Box uses IMGS2 and supports a CUSTOM filename fallback. */
  filename_mirror=0; box_custom=1; opens=closes=seeks=0;
  put32(18,80); put32(22,(uint32_t)-80); put32(34,80*160);
  assert(artwork_load("/Game.gba",1,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(opens==1 && closes==1 && seeks==0 && image_reads==6 && info.width==80);
  assert(!strcmp(last_path,"/SYSTEM/IMGS2/CUSTOM/Game.bmp"));
  opens=closes=0;
  unsigned reads_before_oversize = image_reads;
  assert(!artwork_load("/Game.gba",1,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE-1,&info));
  assert(opens==2 && closes==1 && image_reads==reads_before_oversize);

  /* Original EZ-FLASH Title art uses the ROM game code at 0xAC. */
  box_custom=0; gamecode_header=1; gamecode_art=1; opens=closes=seeks=0;
  put32(18,120); put32(22,(uint32_t)-80); put32(34,120*160);
  assert(artwork_load("/Game Boy Advance/P/Pokemon FireRed.gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(opens==4 && closes==2 && seeks==1 && image_reads==8 && info.width==120);
  assert(!strcmp(last_path,"/SYSTEM/IMGS/B/P/BPRE.bmp"));
  gamecode_art=0; gamecode_flat_art=1; opens=closes=seeks=0;
  assert(artwork_load("/Game Boy Advance/P/Pokemon FireRed.gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(opens==5 && closes==2 && seeks==1 && image_reads==10 && info.width==120);
  assert(!strcmp(last_path,"/SYSTEM/IMGS/BPRE.bmp"));
  /* Original EZ fixed-record reader ignores inconsistent desktop metadata.
   * Exercise this only after CUSTOM/filename misses, with a valid code path. */
  put32(28,24); put32(22,80); put32(2,28854); put32(34,28800);
  assert(artwork_load("/Pokemon.gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(info.width==120 && info.height==80 && !info.transparent);
  filename_custom=1;
  assert(!artwork_load("/Game Boy Advance/P/Game Boy game (USA).gbc",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  filename_custom=0;
  put32(28,16); put32(22,(uint32_t)-80); put32(2,sizeof(bmp)); put32(34,19200);
  /* Historical AGS TCHK planes=0 and native EZ records with non-BM prefix
   * or trailing data. No-Intro tags must not change the header lookup key. */
  strcpy(rom_code,"TCHK");bmp[26]=0;bmp[27]=0;gamecode_art=1;
  assert(artwork_load("/AGS Aging Cartridge.gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(!strcmp(last_path,"/SYSTEM/IMGS/T/C/TCHK.bmp"));
  gamecode_art=0;
  strcpy(rom_code,"BPRE");bmp[26]=1;memcpy(bmp+6,"DST1",4);
  assert(artwork_load("/Pokemon.gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(info.transparent);
  image_size=sizeof(bmp)+128;put32(2,image_size);
  assert(artwork_load("/Pokemon (USA) (En).gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(info.transparent); /* DE DST1 ignores declared/trailing file size. */
  image_size=sizeof(bmp);put32(2,image_size);
  memset(bmp+6,0,4);bmp[0]=0;image_size=sizeof(bmp)+128;
  const char *tagged="/Game Boy Advance/P/Pokemon FireRed (USA) (En,Fr,De).gba";
  assert(artwork_load(tagged,0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(!strcmp(last_rom_path,tagged) && !strcmp(last_path,"/SYSTEM/IMGS/BPRE.bmp"));
  image_size=sizeof(bmp)-1;
  assert(!artwork_load(tagged,0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  image_size=sizeof(bmp);bmp[0]='B';bmp[26]=1;bmp[27]=0;bmp[28]=16;
  gamecode_header=gamecode_flat_art=0;

  /* All tail lengths, including an odd valid BMP offset and padding guard. */
  box_custom=1;
  for(unsigned offset=54;offset<=57;++offset) {
    image_size=offset+80*160;
    put32(2,image_size);put32(10,offset);put32(18,80);put32(34,80*160);
    unsigned cap=(image_size+1)&~1u;
    assert(artwork_load("/Game.gba",1,pixels,sizeof(pixels),bmp_buffer,cap,&info));
    assert(info.width==80);
    if(image_size&1) {
      unsigned before_padding=image_reads;
      assert(!artwork_load("/Game.gba",1,pixels,sizeof(pixels),bmp_buffer,image_size,&info));
      assert(image_reads==before_padding);
    }
  }
  box_custom=0;image_size=sizeof(bmp);
  put32(2,image_size);put32(10,54);put32(18,120);put32(34,120*160);
  filename_custom=1;
  unsigned before_alignment=image_reads;
  assert(!artwork_load("/Game Boy Advance/P/Game Boy game (USA).gba",0,pixels,sizeof(pixels),bmp_buffer+1,BMP_BUFFER_SIZE-1,&info));
  assert(image_reads==before_alignment);
  fail_read=1;
  assert(!artwork_load("/Game Boy Advance/P/Game Boy game (USA).gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  fail_read=0; fail_close=1;
  assert(!artwork_load("/Game Boy Advance/P/Game Boy game (USA).gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  fail_close=0;
  filename_custom=1; fail_tail=1;
  assert(!artwork_load("/Game Boy Advance/P/Game Boy game (USA).gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  fail_tail=0; filename_custom=0;
  opens=closes=0; fail_read=1;
  assert(!artwork_load("/Game.gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  fail_read=0; fail_seek=1;
  assert(!artwork_load("/Game.gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  fail_seek=0; fail_close=1;
  assert(!artwork_load("/Game.gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  fail_close=0; bmp[0]='X';
  assert(!artwork_load("/Game.gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(artwork_directory_path(path,sizeof(path),"/Game Boy Advance",0));
  assert(!strcmp(path,"/SYSTEM/IMGS/CUSTOM/Game Boy Advance.bmp"));
  assert(artwork_directory_path(path,sizeof(path),"/Collections/Game.Boy Advance",1));
  assert(!strcmp(path,"/SYSTEM/IMGS2/CUSTOM/Game.Boy Advance.bmp"));
  assert(!artwork_directory_path(path,sizeof(path),"/..",0));
  assert(!artwork_directory_path(path,sizeof(path),"/A/../Folder",0));
  assert(!artwork_directory_path(path,12,"/Game Boy Advance",0));
  unsigned before_folder_rom=rom_opens;
  memcpy(bmp,"BM",2);put32(18,120);put32(34,19200);folder_title=true;
  memcpy(bmp+6,"DST1",4);
  assert(artwork_load_directory("/Game Boy Advance",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(info.width==120 && info.transparent && rom_opens==before_folder_rom);
  assert(!strcmp(last_path,"/SYSTEM/IMGS/CUSTOM/Game Boy Advance.bmp"));
  folder_box=true;put32(18,80);put32(34,12800);
  assert(artwork_load_directory("/Game Boy Advance",1,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(info.width==80 && !strcmp(last_path,"/SYSTEM/IMGS2/CUSTOM/Game Boy Advance.bmp"));
  assert(!artwork_load_directory("/Game Boy Advance",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  folder_box=folder_title=false;
  assert(!artwork_load_directory("/Game Boy Advance",1,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(rom_opens==before_folder_rom);
  /* NOR can resolve Title art from metadata even after its SD ROM is removed. */
  put32(18,120);put32(22,(uint32_t)-80);put32(34,19200);
  bmp[26]=1;bmp[27]=0;put32(28,16);put32(30,0);
  gamecode_art=1;gamecode_flat_art=0;
  assert(!artwork_load_named("/Removed.gba",0,pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(rom_opens==before_folder_rom);
  assert(artwork_load_gamecode("BPRE",pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(!strcmp(last_path,"/SYSTEM/IMGS/B/P/BPRE.bmp") && info.width==120);
  gamecode_art=0;gamecode_flat_art=1;
  assert(artwork_load_gamecode("BPRE",pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(!strcmp(last_path,"/SYSTEM/IMGS/BPRE.bmp"));
  assert(!artwork_load_gamecode("../X",pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  fail_close=1;
  assert(!artwork_load_gamecode("BPRE",pixels,sizeof(pixels),bmp_buffer,BMP_BUFFER_SIZE,&info));
  assert(rom_opens==before_folder_rom);
  return 0;
}
