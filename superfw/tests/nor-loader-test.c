/* NOR writer replay: simulated file/flash, never accesses a physical cartridge. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_GBA_ROM_SIZE (32u*1024*1024)
#define NOR_BLOCK_SIZE (4u*1024*1024)
#define DIRSAVE_REQ_SPACE 1024u
#define FLASH_DIRSAV_PAYLOAD_W0 0
#define FLASH_IGM_TRAMPOLINE_W0 0
#define GBA_ROM_BASE_WS1 0x0a000000u
#define LOAD_BS 8192u
#define ERR_FLASH_OP 1
#define ERR_LOAD_BADROM 2
#define FR_OK 0
#define FA_READ 1
#define MIN(a,b) ((a)<(b)?(a):(b))
typedef unsigned UINT;
typedef int FRESULT;
typedef struct {unsigned pos;} FIL;
typedef struct {uint8_t bytes[192];} t_rom_header;
typedef struct {unsigned hole_addr,hole_size;} t_patch;
typedef int t_flash_erase_state;
typedef void (*progress_fn)(unsigned,unsigned);
static struct {unsigned size,blksize,blkcount,blkwrite;} flashinfo={128*1024*1024,65536,2048,32};
static const uint8_t directsave_payload[16]={0},ingame_trampoline_payload[16]={0};
static const unsigned directsave_payload_size=16,ingame_trampoline_payload_size=16;
static bool use_verify_nor=true;
static unsigned fs,closed,erased,programmed,mapped,short_at;
static bool close_error,read_error;
static int f_open(FIL *f,const char *name,int flags) {(void)name;(void)flags;f->pos=0;return 0;}
static unsigned f_size(FIL *f) {(void)f;return fs;}
static int f_close(FIL *f) {(void)f;closed++;return close_error;}
static int f_read(FIL *f,void *buf,unsigned count,UINT *rd) {
 if(read_error)return 1;
 *rd=MIN(count,fs-f->pos);if(f->pos>=short_at&&*rd)(*rd)--;
 for(unsigned i=0;i<*rd;i++)((uint8_t*)buf)[i]=(uint8_t)((f->pos+i)*37u+3u);
 f->pos+=*rd;return 0;
}
static void set_superchis_normap(const uint8_t *map) {(void)map;mapped++;}
static void reset_superchis_normap(void) {assert(mapped);mapped--;}
static void dma_memset16(void *dst,unsigned value,unsigned count) {for(unsigned i=0;i<count;i++)((uint16_t*)dst)[i]=value;}
static void dma_memcpy32(void *dst,const void *src,unsigned count) {memcpy(dst,src,count*4);}
static void flash_erase_fsm_start(t_flash_erase_state *st,unsigned addr,unsigned size,unsigned count) {(void)st;(void)addr;(void)size;(void)count;erased++;}
static int flash_erase_fsm_step(t_flash_erase_state *st) {(void)st;return 1;}
static void patch_apply_rom(void *a,unsigned b,unsigned c,bool d,const t_patch *e,bool f,unsigned g,unsigned h) {(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;}
static void payload_apply_rom(void *a,unsigned b,unsigned c,const void *d,unsigned e,unsigned f) {(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;}
static bool flash_verify(unsigned addr,const void *data,unsigned size) {(void)addr;(void)data;(void)size;return true;}
static bool flash_program_buffered(unsigned addr,const void *data,unsigned count,unsigned write) {
 (void)write;assert(!(count&3));unsigned off=addr-GBA_ROM_BASE_WS1;
 for(unsigned i=0;i<count;i++)assert(((const uint8_t*)data)[i]==(off+i<fs?(uint8_t)((off+i)*37u+3u):0xff));
 programmed+=count;return true;
}
#include "nor-loader.h"
static void reset(unsigned size) {fs=size;closed=erased=programmed=mapped=0;short_at=UINT32_MAX;read_error=close_error=false;}
int main(void) {
 uint8_t *scratch=malloc(128*1024);assert(scratch);uint8_t map[8]={0};t_rom_header hdr={0};
 reset(10003);assert(!flash_gba_nor("rom",fs,&hdr,NULL,false,false,false,map,NULL,scratch,128*1024));assert(closed==1&&!mapped&&programmed==10004);
 reset(MAX_GBA_ROM_SIZE);assert(!flash_gba_nor("rom",fs,&hdr,NULL,false,false,false,map,NULL,scratch,128*1024));assert(programmed==fs&&closed==1&&!mapped);
 reset(MAX_GBA_ROM_SIZE);assert(flash_gba_nor("rom",fs,&hdr,NULL,true,false,false,map,NULL,scratch,128*1024)==ERR_LOAD_BADROM);assert(!erased&&!mapped);
 reset(10003);short_at=8192;assert(flash_gba_nor("rom",fs,&hdr,NULL,false,false,false,map,NULL,scratch,128*1024)==ERR_LOAD_BADROM);assert(closed==1&&!mapped&&!programmed);
 reset(10003);assert(flash_gba_nor("rom",fs+1,&hdr,NULL,false,false,false,map,NULL,scratch,128*1024)==ERR_LOAD_BADROM);assert(closed==1&&!mapped&&!erased);
 reset(10003);read_error=true;assert(flash_gba_nor("rom",fs,&hdr,NULL,false,false,false,map,NULL,scratch,128*1024)==ERR_LOAD_BADROM);assert(closed==1&&!mapped&&!programmed);
 reset(10003);close_error=true;assert(flash_gba_nor("rom",fs,&hdr,NULL,false,false,false,map,NULL,scratch,128*1024)==ERR_LOAD_BADROM);assert(closed==1&&!mapped);
 free(scratch);puts("NOR writer: 32MiB/no-patch, odd tails, exact reads, source size, payload and cleanup guards passed (mock flash).");
}
