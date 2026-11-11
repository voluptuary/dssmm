#!/usr/bin/env python3
"""Production BLUE catalog enumeration with simulated banked ROM headers."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
source=(ROOT/'targets/blue/source/NORflash_OP.c').read_text()
function=source[source.index('u32 IWRAM_CODE GetFileListFromNor('):]
header=(ROOT/'targets/blue/source/ezkernel.h').read_text()
catalog=header[header.index('typedef struct FM_NOR_FILE_SECT'):header.index('typedef struct FM_Folder_SECT')]
PRE=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "blue_nor_policy.h"
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef volatile uint8_t vu8;
typedef volatile uint16_t vu16;
#define IWRAM_CODE
#define MAX_NOR 64
#define NOR_FAT_SIZE 960
#define First_game_address BLUE_NOR_FIRST
#define SYSTEM_MODE_OS 1
static _Alignas(4) unsigned char flash[0x800000];
static unsigned char headers[1024][0xC0];
#define FlashBase_S98 ((uintptr_t)flash)
static u16 REG_IME;
static u32 gl_currentpage, gl_norOffset, last_page, page_calls;
'''
MOCK=r'''
static struct { u32 before; FM_NOR_FS entries[64]; u32 after; } guarded;
#define pNorFS (guarded.entries)
static void Blue_PrepareFlashAccess(void) { assert(last_page==gl_currentpage); }
static void *my_memcpy(void *dst, const void *src, u32 size) {return memmove(dst,src,size);}
static void SetRompage_MODE(u16 page,u16 mode) {
    assert(mode==SYSTEM_MODE_OS && !(page%0x1000) && page<=0xF000 && REG_IME==0);
    last_page=page; page_calls++;
    memset(flash,0xFF,sizeof flash);
    unsigned first=page/0x1000*64;
    for(unsigned row=0;row<64;row++) memcpy(flash+row*0x20000,headers[first+row],0xC0);
}
static void reset(void) {
    memset(&guarded,0,sizeof guarded); memset(headers,0xFF,sizeof headers);
    guarded.before=0x12345678; guarded.after=0x87654321;
    gl_currentpage=gl_norOffset=last_page=page_calls=0; REG_IME=1;
}
static void game(unsigned slot,unsigned page) {
    FM_NOR_FS *entry=&pNorFS[slot];
    entry->rompage=page; entry->filesize=0x40000;
    snprintf(entry->filename,sizeof entry->filename,"game%u.gba",slot);
    snprintf(entry->gamename,sizeof entry->gamename,"GAME%u",slot);
    memset(headers[page],0,0xC0); headers[page][0xB2]=0x96;
    memcpy(headers[page]+0xA0,entry->gamename,sizeof entry->gamename);
}
'''
POST=r'''
int main(void) {
    reset(); assert(GetFileListFromNor()==0 && REG_IME==1 && gl_norOffset==0x800000);
    reset(); REG_IME=0;
    for(unsigned i=0;i<64;i++) game(i,64+i*2);
    assert(GetFileListFromNor()==64 && REG_IME==0 && last_page==0);
    assert(gl_norOffset==0x1800000 && guarded.before==0x12345678 && guarded.after==0x87654321);
    reset(); game(63,1022);
    assert(GetFileListFromNor()==1 && pNorFS[0].rompage==1022 && !pNorFS[63].filesize);
    assert(last_page==0 && REG_IME==1);
    reset(); game(0,64); game(1,64);
    assert(GetFileListFromNor()==1);
    reset(); game(0,64); pNorFS[0].filesize=0xFFFFFFFF;
    game(1,66); pNorFS[1].rompage=0xFFFF;
    game(2,68); memset(pNorFS[2].filename,'x',sizeof pNorFS[2].filename);
    game(3,70); headers[70][0xB2]=0;
    game(4,72); headers[72][0xA0]^=1;
    game(5,74);
    assert(GetFileListFromNor()==1 && pNorFS[0].rompage==74 && REG_IME==1);
    assert(guarded.before==0x12345678 && guarded.after==0x87654321);
    puts("BLUE NOR scan: 64 slots, malformed spans/names, overlap, all banks and IRQ restoration passed (ROM simulated).");
}
'''
with tempfile.TemporaryDirectory() as folder:
    c=Path(folder)/'test.c';binary=Path(folder)/'test'
    c.write_text(PRE+catalog+MOCK+function+POST)
    subprocess.run([os.environ.get('CC','cc'),'-std=gnu11','-O1','-Wall','-Wextra','-Werror',
                    '-fsanitize=address,undefined','-fno-pie','-no-pie',
                    '-I',str(ROOT/'targets/blue/source'),str(c),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
