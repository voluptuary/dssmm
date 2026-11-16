/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "posix-fatfs-fixture.h"
#include <setjmp.h>
#include "common.h"
#include "patchengine.h"
#include "loader_layout.h"
#define LOAD_BS 8192
#define MAPPED_SDRAM 1
static uint8_t cart[MAX_GBA_ROM_SIZE] __attribute__((aligned(4)));
#define GBA_ROM_ADDR cart
static unsigned REG_WAITCNT,menu_installs;
static bool seek_error;
static uint8_t use_slowld,boot_bios_splash,use_fastew;
static bool slowsd=true,sd=true,writable=true;
static jmp_buf launched;
static const unsigned ingame_menu_runtime_size=216452,directsave_payload_size=4;
static const uint8_t directsave_payload[4]={0};
FRESULT f_lseek(FIL *f,FSIZE_t offset){return seek_error||fseek(handles[f->obj.id],offset,SEEK_SET)?FR_DISK_ERR:FR_OK;}
static void set_supercard_mode(unsigned area,bool write,bool read){assert(area==MAPPED_SDRAM);writable=write;sd=read;}
void rom_copy_write16(void *dst,const void *src,unsigned size){assert(writable&&!sd&&!(size&31));memcpy(dst,src,size);}
static void dma_memcpy32(void *dst,const void *src,unsigned words){assert(writable&&!sd);memcpy(dst,src,words*4);}
static void dma_memset16(void *dst,uint16_t value,uint16_t count){assert(writable&&!sd);uint16_t *p=dst;while(count--)*p++=value;}
static unsigned font_block_size(void){return 696872;}
static void savestate_filename_calc(const char *fn,char *out){(void)fn;strcpy(out,"/SYSTEM/SAVESTATE/Game");}
static bool load_ingame_menu(uint32_t base,uint32_t space,bool useds,const char *save,const char *state,bool rtc,unsigned cheats){
 (void)useds;(void)save;(void)state;(void)rtc;(void)cheats;assert(base>=GBA_ROM_BASE&&base-GBA_ROM_BASE+space<=sizeof(cart));
 memset(cart+base-GBA_ROM_BASE,0xaa,space);++menu_installs;return true;
}
void payload_apply_rom(uint8_t *buffer,unsigned size,uint32_t base,const uint8_t *data,unsigned data_size,uint32_t offset){(void)buffer;(void)size;(void)base;(void)data;(void)data_size;(void)offset;}
bool patch_apply_rom(uint8_t *buffer,unsigned size,uint32_t base,bool waits,const t_patch *p,bool rtc,uint32_t menu,uint32_t dsaddr){(void)buffer;(void)size;(void)base;(void)waits;(void)p;(void)rtc;(void)menu;(void)dsaddr;return true;}
static void load_rtcclock_data(const t_rtc_info *rtc){(void)rtc;}
static void fix_gba_header(uint16_t *header){assert(header==(uint16_t*)cart);}
static void load_directsave_config(const t_dirsave_info *info){(void)info;}
void launch_reset(bool bios,bool fast){assert(!bios&&!fast&&!writable&&!sd);longjmp(launched,1);}
#include "gba-load-under-test.h"
static void closed(void){for(unsigned i=0;i<8;i++)assert(!handles[i]);}
static void load_success(unsigned size,const t_patch *patch,bool menu,unsigned cheats){
 sd=writable=true;memset(cart,0xcc,sizeof(cart));
 if(!setjmp(launched)){assert(load_gba_rom("/test.gba",size,0,patch,0,menu,0,cheats,0)==0);abort();}
 assert(REG_WAITCNT==0x4000&&slowsd);closed();
}
static void sparse_rom(unsigned size){char name[1024];path(name,"/test.gba");FILE *f=fopen(name,"wb");assert(f);assert(!ftruncate(fileno(f),size));assert(!fclose(f));}
int main(void) {
 char temp[]="/tmp/dssmm-gba-loader-XXXXXX";assert(mkdtemp(temp));strcpy(root,temp);sparse_rom(10003);
 for(unsigned slow=0;slow<2;slow++){
  use_slowld=slow;load_success(10003,0,false,0);
  for(unsigned i=0;i<10003;i++)assert(cart[i]==0);
  for(unsigned i=10003;i<11264;i++)assert(cart[i]==0xff);
  assert(cart[11264]==0xcc);
 }
 sd=writable=true;short_read=1;assert(load_gba_rom("/test.gba",10003,0,0,0,false,0,0,0)==ERR_LOAD_BADROM);short_read=0;assert(sd&&slowsd);closed();
 read_error=1;assert(load_gba_rom("/test.gba",10003,0,0,0,false,0,0,0)==ERR_LOAD_BADROM);read_error=0;assert(sd&&slowsd);closed();
 read_error_after=8192;assert(load_gba_rom("/test.gba",10003,0,0,0,false,0,0,0)==ERR_LOAD_RESTART);read_error_after=0;assert(sd&&slowsd);closed();
 close_error=1;assert(load_gba_rom("/test.gba",10003,0,0,0,false,0,0,0)==ERR_LOAD_RESTART);close_error=0;closed();
 assert(load_gba_rom("/test.gba",10004,0,0,0,true,0,0,0)==ERR_LOAD_BADROM);assert(!menu_installs);closed();
 assert(load_gba_rom("/test.gba",10003,0,0,0,true,0,UINT32_MAX,0)==ERR_LOAD_MENU);assert(!menu_installs);closed();
 sparse_rom(MAX_GBA_ROM_SIZE);unsigned needed;assert(loader_menu_size(ingame_menu_runtime_size,font_block_size(),64,&needed));
 t_patch patch={.hole_addr=2000,.hole_size=needed+1024};use_slowld=1;
 load_success(MAX_GBA_ROM_SIZE,&patch,true,64);assert(menu_installs==1);
 for(unsigned i=0;i<MAX_GBA_ROM_SIZE;i++)assert(cart[i]==(i>=patch.hole_addr&&i<patch.hole_addr+patch.hole_size?0xaa:0));
 sd=writable=true;seek_error=true;assert(load_gba_rom("/test.gba",MAX_GBA_ROM_SIZE,0,&patch,0,true,0,64,0)==ERR_LOAD_RESTART);seek_error=false;assert(sd&&slowsd);closed();
 assert(f_unlink("/test.gba")==FR_OK);assert(!rmdir(root));
 puts("GBA loader exact fast/slow tails, append padding, internal IGM/cheat hole, short/error reads and handle closure passed.");
}
