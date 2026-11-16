/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "fatfs/ff.h"
#include "emu.h"
#include "pogo_plugin.h"
#include "config.h"
const char *find_extension(const char *s);
#define LOAD_BS 8192
#define ERR_LOAD_NOEMU 1
#define ERR_LOAD_BADROM 2
#define MAPPED_SDRAM 0
static unsigned char cartridge[14*1024*1024] __attribute__((aligned(4)));
#define GBA_ROM_ADDR cartridge
static unsigned REG_WAITCNT;
static bool use_fastew=false;
typedef void(*progress_fn)(unsigned,unsigned);
static int missing=0,fail_read=0,short_read=0,close_error=0,opens=0,closes=0;
static unsigned available_plugins=1;
FRESULT f_stat(const TCHAR *path,FILINFO *info) {
  const char *suffix=strrchr(path,'.');unsigned bit=!strcmp(suffix,".bin")?1:!strcmp(suffix,".gba")?2:!strcmp(suffix,".mb")?4:8;
  if(!(available_plugins&bit))return FR_NO_FILE;
  assert(!strncmp(path,"/SYSTEM/PLUG/nes.",17) || !strncmp(path,"/SYSTEM/PLUG/sms.",17));
  memset(info,0,sizeof(*info));return FR_OK;
}
static uint32_t ps=257,fs=8193,abi[24];static jmp_buf launched;
FRESULT f_open(FIL *f,const TCHAR *path,BYTE mode) {
  (void)mode;opens++;
  if(strstr(path,"/PLUG/") && (missing || !strstr(path,".bin")))return FR_NO_FILE;
  memset(f,0,sizeof(*f));f->obj.objsize=strstr(path,"/PLUG/")?ps:fs;return FR_OK;
}
FRESULT f_read(FIL *f,void *buf,UINT n,UINT *got) {
  if(fail_read)return FR_DISK_ERR;
  if(n>f_size(f)-f->fptr)n=f_size(f)-f->fptr;
  if(short_read&&n)n--;
  for(unsigned i=0;i<n;i++)((unsigned char*)buf)[i]=(f->fptr+i)*17+3;
  *got=n;f->fptr+=n;return FR_OK;
}
FRESULT f_close(FIL *f){(void)f;closes++;return close_error?FR_DISK_ERR:FR_OK;}
static void set_supercard_mode(int a,bool b,bool c){(void)a;(void)b;(void)c;}
static void rom_copy_write16(void *dst,const void *src,unsigned n) {
  assert(!((uintptr_t)dst&1));assert(!((uintptr_t)src&1));assert(!(n&31));
  assert((unsigned char*)dst>=cartridge && (unsigned char*)dst+n<=cartridge+sizeof(cartridge));
  memcpy(dst,src,n);
}
static void memcpy32(void *dst,const void *src,unsigned n){assert(dst==(void*)0x0203fbf8);assert(n==96);memcpy(abi,src,n);}
void launch_pogo_reset(bool fast,unsigned mode,unsigned size){assert(!fast && !mode && size==ps);longjmp(launched,1);}
#include "pogo-loader-under-test.h"
int main(void) {
  char resolved[64];
  assert(find_pogo_plugin("/Game.NES",resolved)&&!strcmp(resolved,"/SYSTEM/PLUG/nes.bin"));
  available_plugins=14;assert(find_pogo_plugin("/Game.NES",resolved)&&!strcmp(resolved,"/SYSTEM/PLUG/nes.gba"));
  available_plugins=12;assert(find_pogo_plugin("/Game.NES",resolved)&&!strcmp(resolved,"/SYSTEM/PLUG/nes.mb"));
  available_plugins=8;assert(find_pogo_plugin("/Game.NES",resolved)&&!strcmp(resolved,"/SYSTEM/PLUG/nes.mbz"));
  available_plugins=0;assert(!find_pogo_plugin("/Game.NES",resolved));
  assert(!find_pogo_plugin("/no-extension",resolved));available_plugins=1;
  assert(find_pogo_plugin("/Sonic.GG",resolved)&&!strcmp(resolved,"/SYSTEM/PLUG/sms.bin"));
  assert(find_pogo_plugin("/Sonic.SMS",resolved)&&!strcmp(resolved,"/SYSTEM/PLUG/sms.bin"));
  const t_emu_loader *gg=get_emu_info("gg"),*sms=get_emu_info("sms");
  for(unsigned i=0;gg[i].emu_name;i++)assert(!strcmp(gg[i].emu_name,sms[i].emu_name));
  uint8_t header[64] __attribute__((aligned(4)));
  assert(gg[0].hndlr(header,"/Sonic.gg",1234)==40 && header[8]==1);
  assert(sms[0].hndlr(header,"/Sonic.sms",1234)==40 && header[8]==0);
  gg[1].hndlr(header,"/Sonic.gg",1234);assert(header[8]==4);
  sms[1].hndlr(header,"/Sonic.sms",1234);assert(header[8]==0);
  uint32_t startup[16]={0xe3a00014,0xe59f1008,0xe1c100b0,0,0,0x04000204};
  assert(pogo_fix_waitcnt(startup,sizeof(startup)) && startup[0]==0xe3a00901);
  assert(!pogo_fix_waitcnt(startup,sizeof(startup)));
  startup[0]=0xe3a00014;startup[5]=0x04000208;
  assert(!pogo_fix_waitcnt(startup,sizeof(startup)) && startup[0]==0xe3a00014);
  startup[5]=0x04000204;assert(!pogo_fix_waitcnt(startup,20));
  startup[1]=0xe59f1009;assert(!pogo_fix_waitcnt(startup,sizeof(startup)));
  startup[1]=0xe59f1fff;assert(!pogo_fix_waitcnt(startup,sizeof(startup)));
  uint32_t fo,to,ram[24],table[22];
  assert(pogo_layout(257,8193,&fo,&to)&&fo==260&&to==32768);
  assert(!pogo_layout(0,1,&fo,&to));assert(!pogo_layout(1,0,&fo,&to));
  assert(!pogo_layout(0xffffffff,1,&fo,&to));assert(!pogo_layout(1,0xffffffff,&fo,&to));
  assert(!pogo_layout(14*1024*1024,1,&fo,&to));
  assert(pogo_layout(257,8193,&fo,&to));
  pogo_arguments(ram,table,"/SYSTEM/PLUG/nes.bin","/Games/A game (USA).nes",fo,8193,to);
  assert(ram[0]==8193&&ram[1]==0x08000104&&ram[2]==0xfab0babe&&ram[3]==2);
  assert(!strcmp((char*)&ram[4],"/nes.bin"));
  assert(!strcmp((char*)&ram[4]+9,"/A game (USA).nes"));
  assert(table[0]==ram[2]&&table[1]==0x80000050&&table[10]==260);
  assert(!strcmp((char*)(table+2),"nes.bin")&&!strcmp((char*)(table+12),"A game (USA).nes"));
  assert(table[11]+0x08008008==0x08000000&&table[21]+0x08008008==ram[1]);
  pogo_arguments(ram,table,"nes.gba","/a-very-long-file-name-with-many-extra-characters.nes",fo,8193,to);
  assert(strlen((char*)(table+12))==31);assert(!strcmp((char*)(table+12)+27,".nes"));
  uint8_t lz[]={0x10,4,0,0,0,1,2,3,4};
  assert(pogo_ram_valid(lz,sizeof(lz),2));
  assert(!pogo_ram_valid(lz,sizeof(lz)-1,2));
  lz[4]=0x80;assert(!pogo_ram_valid(lz,sizeof(lz),2));
  assert(pogo_ram_valid(lz,4,1));assert(!pogo_ram_valid(lz,3,1));
  assert(!pogo_ram_valid(lz,0x3fbfc,1));
  memset(cartridge,0xcc,sizeof(cartridge));
  if(!setjmp(launched)){load_pogo_plugin("/Game.NES",fs,NULL);assert(0);}
  assert(REG_WAITCNT==0x4000&&abi[0]==fs&&abi[1]==0x08000104);
  for(unsigned i=0;i<ps;i++)assert(cartridge[i]==(unsigned char)(i*17+3));
  for(unsigned i=ps;i<fo;i++)assert(cartridge[i]==0);
  for(unsigned i=0;i<fs;i++)assert(cartridge[fo+i]==(unsigned char)(i*17+3));
  assert(cartridge[fo+fs]==0&&cartridge[fo+fs+3]==0xcc);
  uint32_t *loaded=(uint32_t*)(cartridge+to);
  assert(loaded[0]==0xfab0babe&&loaded[20]==fs&&loaded[21]+0x08008008==abi[1]);
  missing=1;assert(load_pogo_plugin("/Game.nes",fs,NULL)==ERR_LOAD_NOEMU);missing=0;
  fail_read=1;assert(load_pogo_plugin("/Game.nes",fs,NULL)==ERR_LOAD_BADROM);fail_read=0;
  short_read=1;assert(load_pogo_plugin("/Game.nes",fs,NULL)==ERR_LOAD_BADROM);short_read=0;
  close_error=1;assert(load_pogo_plugin("/Game.nes",fs,NULL)==ERR_LOAD_BADROM);close_error=0;
  assert(load_pogo_plugin("/Game.nes",0xffffffff,NULL)==ERR_LOAD_BADROM);
  puts("Pogoshell layout, arguments, exact/tail cartridge transfers and read/close failures passed.");
}
