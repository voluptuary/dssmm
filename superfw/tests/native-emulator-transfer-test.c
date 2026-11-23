/* Production legacy-emulator transfer: real file reads, exact tails and RAM headers. */
#include "posix-fatfs-fixture.h"
#include "emu.h"
#include <stdint.h>
#include <setjmp.h>
typedef void(*progress_fn)(unsigned,unsigned);
#define ERR_LOAD_BADROM 1
#define ERR_LOAD_NOEMU 4
#define MAPPED_SDRAM 1
#define LOAD_BS 8192
#ifndef EMU_HEADER_MAX_BYTES
#define EMU_HEADER_MAX_BYTES 64
#endif
static uint32_t cart_words[(14*1024*1024+128)/4],core[16];
#define GBA_ROM_ADDR ((uint8_t*)cart_words)
static bool sd_mapped=true,use_slowld,use_fastew;
static uint16_t waitcnt;
#define REG_WAITCNT waitcnt
static jmp_buf launched;
const t_emu_loader pogo_plugin_loader={"pogo:",0};
static unsigned load_pogo_plugin(const char *a,unsigned b,void(*c)(unsigned,unsigned)){(void)a;(void)b;(void)c;return ERR_LOAD_NOEMU;}
static void set_supercard_mode(unsigned mode,bool writable,bool sd){(void)writable;assert(mode==MAPPED_SDRAM);sd_mapped=sd;}
static const void *get_vfile_ptr(const char *tag){assert(!sd_mapped);return !strcmp(tag,"TEST")?core:0;}
static unsigned upkr_unpack16(void *dst,const void *src){assert(!sd_mapped);memcpy(dst,src,sizeof(core));return sizeof(core);}
static void rom_copy_write16(void *dst,const void *src,unsigned size){assert(!sd_mapped);memcpy(dst,src,size);}
static void dma_memcpy32(void *dst,const void *src,unsigned words){assert(!sd_mapped);memcpy(dst,src,words*4);}
static void rom_write16_exact(void *dst,const void *src,unsigned size){assert(!(size&1));rom_copy_write16(dst,src,size);}
static void launch_reset(bool hard,bool fast){assert(!hard&&fast==use_fastew&&!sd_mapped&&waitcnt==0x4000);longjmp(launched,1);}
static unsigned make_header(uint8_t *dst,const char *name,unsigned size){(void)name;(void)size;
 assert((uintptr_t)dst<(uintptr_t)GBA_ROM_ADDR||(uintptr_t)dst>=(uintptr_t)GBA_ROM_ADDR+sizeof(cart_words));
 memset(dst,0x5a,64);return 64;}
const t_emu_loader *get_emu_info(const char *extension){(void)extension;return 0;}
#define POGO_BACKUP_IMAGE ((const uint32_t*)0)
#define POGO_STUB_IMAGE ((const uint32_t*)0)
#define POGO_RETURN_STUB 0x09a00000u
#define VERSION_SLUG_WORD 0
#define FW_MAX_SIZE_KB 2048
static bool pogo_return_available(const uint32_t *a,const uint32_t *b,uint32_t c,uint32_t d){(void)a;(void)b;(void)c;(void)d;assert(!"Non-PCE loader reached return adapter");return false;}
static unsigned pogo_adapt_image(uint32_t *a,uint32_t b,uint32_t c){(void)a;(void)b;(void)c;assert(!"Non-PCE loader adapted its payload");return 0;}
#include "native-emulator.h"
static void no_handles(void){for(unsigned i=0;i<8;i++)assert(!handles[i]);}
static void binary_file(const char *name,unsigned size){char q[1024];path(q,name);FILE *f=fopen(q,"wb");assert(f);for(unsigned i=0;i<size;i++)assert(fputc((i*31+7)&255,f)!=EOF);assert(!fclose(f));}
int main(void){char tmp[]="/tmp/dssmm-native-emu-XXXXXX";assert(mkdtemp(tmp));strcpy(root,tmp);for(unsigned i=0;i<16;i++)core[i]=0x12340000+i;
 const t_emu_loader native={"vfs:TEST",make_header};
 unsigned sizes[]={1,3,512,8193,131584};
 for(volatile unsigned slow=0;slow<2;slow++)for(volatile unsigned n=0;n<5;n++){
  unsigned size=sizes[n];binary_file("/Game.pce",size);memset(cart_words,0xa5,sizeof(cart_words));sd_mapped=true;use_slowld=slow;
  if(!setjmp(launched)){(void)load_extemu_rom("/Game.pce",size,&native,0);assert(!"Valid native launch returned");}
  no_handles();assert(!memcmp(GBA_ROM_ADDR,core,64));for(unsigned i=64;i<128;i++)assert(GBA_ROM_ADDR[i]==0x5a);
  for(unsigned i=0;i<size;i++)assert(GBA_ROM_ADDR[128+i]==((i*31+7)&255));
  for(unsigned i=(size+3)&~3u;i<size+32;i++)assert(GBA_ROM_ADDR[128+i]==0xa5);
  for(unsigned i=size;i<((size+3)&~3u);i++)assert(GBA_ROM_ADDR[128+i]==0);
  assert(f_unlink("/Game.pce")==FR_OK);
 }
 assert(f_mkdir("/SYSTEM")==FR_OK);assert(f_mkdir("/SYSTEM/PLUG")==FR_OK);
 const t_emu_loader external={"Test",make_header};
 assert(load_extemu_rom("/Game.pce",3,&external,0)==ERR_LOAD_NOEMU);no_handles();
 binary_file("/SYSTEM/PLUG/Test.gba",7);binary_file("/Game.pce",3);
 for(volatile unsigned slow=0;slow<2;slow++){
  use_slowld=slow;sd_mapped=true;memset(cart_words,0xa5,sizeof(cart_words));
  if(!setjmp(launched)){(void)load_extemu_rom("/Game.pce",3,&external,0);assert(!"External native launch returned");}
  no_handles();for(unsigned i=0;i<7;i++)assert(GBA_ROM_ADDR[i]==((i*31+7)&255));assert(GBA_ROM_ADDR[7]==0);
  for(unsigned i=8;i<72;i++)assert(GBA_ROM_ADDR[i]==0x5a);
  for(unsigned i=0;i<3;i++)assert(GBA_ROM_ADDR[72+i]==((i*31+7)&255));
  assert(GBA_ROM_ADDR[75]==0&&GBA_ROM_ADDR[76]==0xa5);
 }
 assert(f_unlink("/Game.pce")==FR_OK);assert(f_unlink("/SYSTEM/PLUG/Test.gba")==FR_OK);
 char external_dir[1024];path(external_dir,"/SYSTEM/PLUG");assert(!rmdir(external_dir));path(external_dir,"/SYSTEM");assert(!rmdir(external_dir));
 assert(load_extemu_rom("/missing.pce",0,&native,0)==ERR_LOAD_BADROM);no_handles();
 assert(load_extemu_rom("/missing.pce",8*1024*1024+1,&native,0)==ERR_LOAD_BADROM);no_handles();
 binary_file("/Game.pce",8193);sd_mapped=true;
 assert(load_extemu_rom("/Game.pce",8192,&native,0)==ERR_LOAD_BADROM);no_handles();
 read_error=1;assert(load_extemu_rom("/Game.pce",8193,&native,0)==ERR_LOAD_BADROM);read_error=0;no_handles();
 short_read=1;assert(load_extemu_rom("/Game.pce",8193,&native,0)==ERR_LOAD_BADROM);short_read=0;no_handles();
 close_error=1;assert(load_extemu_rom("/Game.pce",8193,&native,0)==ERR_LOAD_BADROM);close_error=0;no_handles();
 assert(f_unlink("/Game.pce")==FR_OK);assert(!rmdir(root));
 puts("Native emulator transfer: RAM-staged headers, both copy modes, exact/odd/headered tails, size/read/close failures and handle cleanup passed.");
}
