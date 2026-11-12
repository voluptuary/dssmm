#!/usr/bin/env python3
"""Exercise real BLUE flash wrappers' IRQ/bank restoration with stable-read mocks."""
from pathlib import Path
import os
import re
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
nor=(ROOT/'targets/blue/source/NORflash_OP.c').read_text()
card=(ROOT/'targets/blue/source/Ezcard_OP.c').read_text()

def body(source,name):
    match=re.search(r'^(?:void|u16|u8)\s+IWRAM_CODE\s+'+name+r'\([^\n]*\)[^\n]*\n\{',source,re.M)
    assert match,name
    start=source.index('{',match.start());depth=0
    for index in range(start,len(source)):
        if source[index]=='{':depth+=1
        elif source[index]=='}':
            depth-=1
            if not depth:return source[match.start():index+1]
    raise AssertionError(name)

functions=[body(card,'Read_S98NOR_ID'),body(card,'Save_info_2_NOR')]
functions += [body(nor,name) for name in ('Chip_Reset','BlueVendor_Block_Erase','WriteFlash_with64word',
                                        'BlueVendor_WriteFlash_with32word','BlueVendor_WriteFlash','Blue_VerifyNorReadback')]
PRE=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef volatile uint16_t vu16;
#define IWRAM_CODE
#define SYSTEM_MODE_OS 1
#define SAVE_info_offset 0x780000
#define NOR_info_offset 0x7A0000
#define SET_info_offset 0x7C0000
#define NOR_FAT_offset 0x7E0000
#define NOR_end_address 0x8000000
static _Alignas(4) u8 flash[0x800000];
#define FlashBase_S98 ((uintptr_t)flash)
static volatile u16 REG_IME;
static u16 gl_currentpage, current_page;
static unsigned prepared, bank_calls;
static void Blue_PrepareFlashAccess(void) {assert(current_page==gl_currentpage);prepared++;}
static void SetRompage_MODE(u16 page,u16 mode) {
    assert(REG_IME==0 && mode==SYSTEM_MODE_OS);current_page=page;bank_calls++;
}
'''
POST=r'''
static void done(unsigned incoming) {
    assert(REG_IME==incoming && current_page==gl_currentpage && prepared);
    prepared=bank_calls=0;
}
int main(void) {
    u16 words[32];for(unsigned i=0;i<32;i++)words[i]=i+1;
    for(unsigned incoming=0;incoming<=1;incoming++) {
        REG_IME=incoming;gl_currentpage=current_page=0;
        (void)Read_S98NOR_ID();assert(!bank_calls);done(incoming);
        assert(Save_info_2_NOR(0x780000,words,sizeof words));done(incoming);
        assert(!Save_info_2_NOR(0,words,sizeof words) && !prepared);
        assert(!Save_info_2_NOR(0x780000,words,3) && !prepared);
        memset(flash+0x20000,0xFF,64);
        assert(Blue_VerifyNorReadback(0x820000,0,64));done(incoming);
        flash[0x20000]=0;assert(!Blue_VerifyNorReadback(0x820000,0,64));done(incoming);
        memcpy(flash+0x20000,words,sizeof words);
        assert(Blue_VerifyNorReadback(0x4020000,(u8 *)words,sizeof words));done(incoming);
        flash[0x20003]^=1;
        assert(!Blue_VerifyNorReadback(0x4020000,(u8 *)words,sizeof words));done(incoming);
        assert(!Blue_VerifyNorReadback(0x7FFFFFE,0,4) && !prepared);
        assert(!Blue_VerifyNorReadback(0xFFFFFE,0,4) && !prepared);
        Chip_Reset();done(incoming);
        BlueVendor_Block_Erase(0x800000);done(incoming);
        BlueVendor_Block_Erase(0x4000000);done(incoming);
        BlueVendor_Block_Erase(0x7FE0000);done(incoming);
        WriteFlash_with64word(0x800000,(u8 *)words,sizeof words);done(incoming);
        BlueVendor_WriteFlash_with32word(0x4000000,(u8 *)words,sizeof words);done(incoming);
        BlueVendor_WriteFlash(0x800000,(u8 *)words,sizeof words);done(incoming);
    }
    puts("BLUE flash guards: audio preparation precedes bank changes; all raw routines preserve incoming IRQ state and restore OS page (stable-read mock).");
}
'''
with tempfile.TemporaryDirectory() as folder:
    c=Path(folder)/'test.c';binary=Path(folder)/'test'
    c.write_text(PRE+'\n'.join(functions)+POST)
    subprocess.run([os.environ.get('CC','cc'),'-std=gnu11','-O1','-Wall','-Wextra','-Werror',
                    '-fsanitize=address,undefined','-fno-pie','-no-pie',str(c),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
