#!/usr/bin/env python3
"""Exercise BLUE allocation policy and actual NOR file-loader I/O ordering."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
source = (ROOT / 'targets/blue/source/NORflash_OP.c').read_text()
start = source.index('u32 BlueVendor_Loadfile2NOR(')
end = source.index('void* IWRAM_CODE my_memcpy', start)
function = source[source.index('u8 Blue_NorCatalogMatchesMap('):source.index('//-----------------------------------------------------------',source.index('void Check_NOR_flash('))] + source[start:end]
header = (ROOT / 'targets/blue/source/ezkernel.h').read_text()
catalog = header[header.index('typedef struct FM_NOR_FILE_SECT'):header.index('typedef struct FM_Folder_SECT')]

PRE = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "blue_nor_policy.h"
#include "nor_write_result.h"
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t FSIZE_t;
typedef unsigned UINT;
#define NOR_FAT_SIZE 960
#define MAX_NOR 64
#define EWRAM_BSS
#define MAX_pReadCache_size 0x20000
#define First_game_address BLUE_NOR_FIRST
#define NOR_end_address BLUE_NOR_END
#define NOR_ID 0x223D
#define PSRAM_CAPACITY_BYTES 0x4000000
#define FA_READ 1
#define FR_OK 0
typedef struct { FSIZE_t size; u32 position; } FIL;
typedef char TCHAR;
static u8 NOR_FAT_buffer[NOR_FAT_SIZE];
static u8 pReadCache[MAX_pReadCache_size];
static unsigned game_total_NOR, gl_norOffset, iTrimSize, initial_count;
static unsigned gl_reset_on, gl_rts_on, gl_sleep_on, gl_cheat_on;
static uint64_t file_size;
static unsigned opened, closed, reads, seeks, erased, programmed, published, progress;
static unsigned fail_read, short_read, fail_seek;
static int fail_open, fail_close, bad_chip, fail_erase, fail_program;
'''
MOCK = r'''
static FM_NOR_FS pNorFS[MAX_NOR];
static unsigned Read_S98NOR_ID(void) { return bad_chip ? 0 : NOR_ID; }
#define f_size(file) ((file)->size)
static int f_open(FIL *file, const char *name, unsigned mode) {
    (void)name; (void)mode; opened++; file->size=file_size; file->position=0; return fail_open;
}
static int f_lseek(FIL *file, unsigned pos) {
    file->position=pos; seeks++; return seeks==fail_seek ? 1 : 0;
}
static int f_read(FIL *file, void *buffer, unsigned bytes, UINT *received) {
    reads++; *received=0;
    if(reads==fail_read) return 1;
    unsigned count=bytes;
    if(file->size-file->position<count) count=file->size-file->position;
    if(reads==short_read && count) count--;
    for(unsigned i=0;i<count;i++) ((u8 *)buffer)[i]=(u8)((file->position+i)^0x5A);
    *received=count; file->position+=count; return 0;
}
static int f_close(FIL *file) { (void)file; closed++; return fail_close; }
static void Read_NORgame_info(void) {}
static void Read_NOR_FAT_info(void) {}
static unsigned GetFileListFromNor(void) { return initial_count; }
static unsigned Find_NOR_space(unsigned bytes) { return BlueNorFindFree(NOR_FAT_buffer,bytes); }
static void Launcher_DrawNORWriteBackground(void) {}
static void Chip_Reset(void) {}
static void PatchInternal(u32 *buffer,unsigned size,unsigned offset) {(void)buffer;(void)size;(void)offset;}
static void GBApatch_NOR(u32 *buffer,unsigned size,unsigned offset,unsigned page)
{(void)buffer;(void)size;(void)offset;(void)page;}
static void GBApatch_Cleanrom_NOR(u32 *buffer,unsigned offset) {(void)buffer;(void)offset;}
static u8 Block_Erase(unsigned address) {
    assert(address>=First_game_address && address<NOR_end_address && !(address%0x20000)); erased++; return !fail_erase;
}
static u8 Blue_ProgramNorBlock(unsigned address,u8 *buffer,unsigned size) {
    assert(size==MAX_pReadCache_size && address+size<=NOR_end_address);
    unsigned offset=programmed*size;
    for(unsigned i=0;i<size;i++) assert(buffer[i]==(offset+i<file_size ? (u8)((offset+i)^0x5A) : 0xFF));
    assert(erased==programmed+1); programmed++; return !fail_program;
}
static void Blue_DrawNorProgress(unsigned complete,unsigned total) {
    assert(complete<=total && complete==programmed); progress++;
}
static unsigned Blue_CommitNorCatalog(void) {
    assert(closed==1); published++; return 1;
}
'''
POST = r'''
static void reset(uint64_t size) {
    file_size=size; game_total_NOR=initial_count=0; gl_norOffset=0; iTrimSize=0;
    gl_reset_on=gl_rts_on=gl_sleep_on=gl_cheat_on=0;
    opened=closed=reads=seeks=erased=programmed=published=progress=0;
    fail_read=short_read=fail_seek=0; fail_open=fail_close=bad_chip=fail_erase=fail_program=0;
    memset(NOR_FAT_buffer,0,sizeof NOR_FAT_buffer);
    memset(pNorFS,0,sizeof pNorFS); memset(pReadCache,0x77,sizeof pReadCache);
}
int main(void) {
    unsigned char map[BLUE_NOR_MAP_SIZE], unchanged[BLUE_NOR_MAP_SIZE];
    memset(map,0,sizeof map);
    assert(BlueNorFindFree(map,0x40000)==0x800000);
    assert(BlueNorMark(map,64,0x40000));
    memcpy(unchanged,map,sizeof map);
    assert(!BlueNorMark(map,64,0x40000) && !memcmp(map,unchanged,sizeof map));
    assert(BlueNorFindFree(map,0x40000)==0x840000);
    assert(!BlueNorValidSpan(63,0x40000) && !BlueNorValidSpan(0x10000,0x40000));
    assert(!BlueNorValidSpan(1023,0x40000) && !BlueNorValidSpan(64,0));
    assert(BlueNorValidSpan(1022,0x40000));
    memset(map,0xFF,sizeof map); map[958]=map[959]=0;
    assert(BlueNorFindFree(map,0x40000)==0x7FC0000);
    assert(!BlueNorFindFree(map,0x80000) && !BlueNorFindFree(map,0x100000000ULL-1));

    unsigned lengths[]={0xC0,0x20000,0x20001,0x40000,0x4000000};
    for(unsigned i=0;i<sizeof lengths/sizeof lengths[0];i++) {
        reset(lengths[i]);
        assert(BlueVendor_Loadfile2NOR("game.gba",0,0,0,0)==0);
        assert(programmed==((lengths[i]+0x3FFFF)&~0x3FFFFu)/0x20000);
        assert(published==1 && closed==1 && progress==programmed);
        assert(pNorFS[0].rompage==64 && pNorFS[0].filesize==programmed*0x20000);
    }
    uint64_t invalid[]={0,0xBF,0x4000001,UINT64_C(0x1000000C0)};
    for(unsigned i=0;i<sizeof invalid/sizeof invalid[0];i++) {
        reset(invalid[i]); assert(BlueVendor_Loadfile2NOR("x",0,0,0,0)==1);
        assert(!erased && !published && closed==1);
    }
    reset(0xC0); bad_chip=1;
    assert(BlueVendor_Loadfile2NOR("x",0,0,0,0)==3 && !opened);
    reset(0xC0); fail_open=1;
    assert(BlueVendor_Loadfile2NOR("x",0,0,0,0)==1 && !closed);
    for(unsigned fault=1;fault<=3;fault++) {
        reset(0x20001); fail_read=fault;
        assert(BlueVendor_Loadfile2NOR("x",0,0,0,0)==1);
        assert(!published && closed==1 && erased==(fault==3 ? 1u : 0u));
        reset(0x20001); short_read=fault;
        assert(BlueVendor_Loadfile2NOR("x",0,0,0,0)==1);
        assert(!published && closed==1 && erased==(fault==3 ? 1u : 0u));
    }
    for(unsigned fault=1;fault<=2;fault++) {
        reset(0xC0); fail_seek=fault;
        assert(BlueVendor_Loadfile2NOR("x",0,0,0,0)==1 && !published && !erased && closed==1);
    }
    reset(0xC0); fail_erase=1;
    assert(BlueVendor_Loadfile2NOR("x",0,0,0,0)==4 && !published && !programmed && closed==1);
    reset(0xC0); fail_program=1;
    assert(BlueVendor_Loadfile2NOR("x",0,0,0,0)==5 && !published && programmed==1 && closed==1);
    reset(0xC0); fail_close=1;
    assert(BlueVendor_Loadfile2NOR("x",0,0,0,0)==1 && !published && closed==1);
    reset(0xC0); initial_count=64;
    assert(BlueVendor_Loadfile2NOR("x",0,0,0,0)==2 && !erased && closed==1);
    reset(0xC0); initial_count=1; pNorFS[0].rompage=64; pNorFS[0].filesize=0x40000; NOR_FAT_buffer[0]=NOR_FAT_buffer[1]=0xFF;
    assert(BlueVendor_Loadfile2NOR("x",0,0,0,0)==0 && pNorFS[1].rompage==66);
    reset(0x40000); iTrimSize=0x40000; gl_reset_on=1;
    assert(BlueVendor_Loadfile2NOR("x",0,1,0,1)==0 && programmed==4);
    reset(0xC0); NOR_FAT_buffer[10]=0xFF;
    assert(BlueVendor_Loadfile2NOR("x",0,0,0,0)==1 && !erased && !published && NOR_FAT_buffer[10]==0xFF);
    reset(0xC0); initial_count=1;pNorFS[0].rompage=64;pNorFS[0].filesize=0x40000;
    assert(BlueVendor_Loadfile2NOR("x",0,0,0,0)==1 && !erased && !published);
    reset(0xC0); memset(pNorFS,0xFF,sizeof pNorFS);memset(NOR_FAT_buffer,0xFF,sizeof NOR_FAT_buffer);
    Check_NOR_flash(); for(unsigned i=0;i<NOR_FAT_SIZE;i++) assert(!NOR_FAT_buffer[i]);
    assert(!erased && !published);
    reset(0xC0);memset(NOR_FAT_buffer,0xFF,sizeof NOR_FAT_buffer);
    Check_NOR_flash();assert(NOR_FAT_buffer[0]==0xFF && !erased && !published);
    unsigned allocation;
    assert(BlueNorAllocation(0x3E010,1,0x40000,&allocation) && allocation==0x80000);
    assert(!BlueNorAllocation(0x40000,1,0x3FFF0,&allocation));
    assert(!BlueNorAllocation(0x4000000,1,0x2000000,&allocation));
    assert(BlueNorAllocation(0x4000000,1,0x1FFE000,&allocation) && allocation==0x4000000);
    assert(!BlueNorAllocation(0xC0,1,0xC1,&allocation));
    reset(0x40000);gl_rts_on=1;
    assert(BlueVendor_Loadfile2NOR("x",0,1,0,0)==1 && !opened && !erased && !published);
    puts("BLUE NOR policy/load: allocation, bounds, exact I/O, padding and publication ordering passed (flash mocked).");
}
'''
with tempfile.TemporaryDirectory() as folder:
    sourcefile=Path(folder)/'test.c'
    binary=Path(folder)/'test'
    sourcefile.write_text(PRE+catalog+MOCK+function+POST)
    subprocess.run([os.environ.get('CC','cc'),'-std=gnu99','-O1','-Wall','-Wextra','-Werror',
                    '-fsanitize=address,undefined','-fno-pie','-no-pie',
                    '-I',str(ROOT/'targets/blue/source'),str(sourcefile),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
