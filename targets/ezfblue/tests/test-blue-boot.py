#!/usr/bin/env python3
"""Validate plugin payload policy and execute linked reset Thumb code with mocked BIOS."""
from pathlib import Path
import os
import struct
import subprocess
import tempfile
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_INTR, UC_HOOK_CODE
from unicorn.arm_const import UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC

ROOT=Path(__file__).resolve().parents[1]
ELF=ROOT/'targets/blue/ezbluekernel-development.elf'
DEVKIT=Path(os.environ['DEVKITARM'])/'bin'

POLICY=r'''
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "kernel_boot_policy.h"
int main(void) {
    assert(KernelPluginRangeFits(0,0x2000000));
    assert(!KernelPluginRangeFits(1,0x2000000));
    assert(!KernelPluginRangeFits(0xFFFFFFFF,2));
    assert(!KernelPluginRangeFits(0x2000000,0x58));
    unsigned char raw[32]={0};
    assert(KernelPluginPayloadValid(raw,0x3FBF8,2));
    assert(!KernelPluginPayloadValid(raw,0x3FBF9,2));
    assert(!KernelPluginPayloadValid(raw,0,2));
    unsigned char literal[]={0x10,4,0,0,0,1,2,3,4};
    assert(KernelPluginPayloadValid(literal,sizeof literal,4));
    assert(!KernelPluginPayloadValid(literal,sizeof literal-1,4));
    unsigned char repeat[]={0x10,4,0,0,0x40,0xA5,0,0};
    assert(KernelPluginPayloadValid(repeat,sizeof repeat,4));
    repeat[6]=0xF0;assert(!KernelPluginPayloadValid(repeat,sizeof repeat,4));
    unsigned char before[]={0x10,3,0,0,0x80,0,0};
    assert(!KernelPluginPayloadValid(before,sizeof before,4));
    literal[0]=0x11;assert(!KernelPluginPayloadValid(literal,sizeof literal,4));
    literal[0]=0x10;literal[1]=0;assert(!KernelPluginPayloadValid(literal,4,4));
    literal[1]=0;literal[2]=0;literal[3]=4;assert(!KernelPluginPayloadValid(literal,4,4));
    unsigned char *fuzz=malloc(4096);assert(fuzz);
    uint32_t seed=1;
    for(unsigned n=0;n<20000;n++) {
        unsigned size=n%4096;
        for(unsigned i=0;i<size;i++) {seed=seed*1664525+1013904223;fuzz[i]=seed>>24;}
        (void)KernelPluginPayloadValid(fuzz,size,4);
    }
    free(fuzz);puts("BLUE plugin payload policy: EWRAM bounds, LZ structure/backreferences and 20,000 malformed cases passed.");
}
'''

def symbols():
    result={}
    for line in subprocess.check_output([str(DEVKIT/'arm-none-eabi-nm'),'-n',str(ELF)],text=True).splitlines():
        parts=line.split()
        if len(parts)==3:result[parts[2]]=int(parts[0],16)
    return result


def run_mode(mode, iwram, entry):
    emulator=Uc(UC_ARCH_ARM,UC_MODE_ARM)
    emulator.mem_map(0x02000000,0x40000)
    emulator.mem_map(0x03000000,0x8000)
    emulator.mem_map(0x04000000,0x1000)
    emulator.mem_map(0x08000000,0x800000)
    emulator.mem_write(0x02000000,b'\xCC'*0x40000)
    emulator.mem_write(0x03000000,iwram)
    payload=b'\xFE\xFF\xFF\xEA\x11\x22\x33\x44'
    compressed=b'\x10\x08\x00\x00\x00'+payload
    emulator.mem_write(0x08000000,compressed if mode==4 else payload)
    emulator.mem_write(0x0203FBF8,struct.pack('<II',123,0x08000000+len(payload)))
    arguments=b'ARGS-PRESERVED'+bytes(100)
    emulator.mem_write(0x0203FC00,arguments)
    emulator.reg_write(UC_ARM_REG_SP,0x0203F000)
    emulator.reg_write(UC_ARM_REG_LR,0x0203EF01)
    emulator.reg_write(UC_ARM_REG_R0,mode)
    events=[]

    def interrupt(uc,number,context):
        pc=uc.reg_read(UC_ARM_REG_PC)
        instruction=int.from_bytes(uc.mem_read(pc-2,2),'little')
        assert instruction>>8==0xDF,(hex(pc),hex(instruction),number)
        swi=instruction&255
        if swi==1:
            flags=uc.reg_read(UC_ARM_REG_R0)
            assert 0x03000000<=uc.reg_read(UC_ARM_REG_SP)<0x03008000
            assert not flags&2,'Reset must preserve the executing IWRAM code'
            if flags&1:uc.mem_write(0x02000000,bytes(0x40000))
            events.append(('clear',flags))
        elif swi==0x11:
            assert uc.reg_read(UC_ARM_REG_R0)==0x08000000 and uc.reg_read(UC_ARM_REG_R1)==0x02000000
            # Fixture is literal-only; policy tests exercise full token structure.
            assert uc.mem_read(0x08000000,len(compressed))==compressed
            uc.mem_write(0x02000000,payload)
            events.append(('lz',0x11))
        elif swi in (0,0x26):
            events.append(('boot',swi));uc.emu_stop()
        else:raise AssertionError(('unexpected BIOS call',swi))

    def payload_entry(uc,address,size,context):
        events.append(('payload',address));uc.emu_stop()

    emulator.hook_add(UC_HOOK_INTR,interrupt)
    emulator.hook_add(UC_HOOK_CODE,payload_entry,begin=0x02000000,end=0x02000001)
    emulator.emu_start(entry|1,0,count=1000000)
    assert events and events[-1][0] in ('boot','payload'),(mode,events)
    assert emulator.mem_read(0x04000208,2)==b'\0\0'
    if mode in (0,6):
        assert ('clear',0xFD) in events
        assert emulator.mem_read(0x0203F000,32)==bytes(32)
        assert events[-1]==('boot',0 if mode==0 else 0x26)
    else:
        assert emulator.mem_read(0x0203FC00,len(arguments))==arguments
        if mode in (2,4):
            assert events[-1]==('payload',0x02000000)
            assert emulator.mem_read(0x02000000,len(payload))==payload
        else:assert events[-1]==('boot',0x26 if mode==1 else 0)
    print(f'BLUE linked boot mode {mode}: {events}')


with tempfile.TemporaryDirectory() as folder:
    c=Path(folder)/'policy.c';binary=Path(folder)/'policy';section=Path(folder)/'iwram.bin'
    c.write_text(POLICY)
    subprocess.run([os.environ.get('CC','cc'),'-std=gnu99','-O1','-Wall','-Wextra','-Werror',
                    '-fsanitize=address,undefined','-fno-pie','-no-pie','-I',str(ROOT/'targets/blue/source'),
                    str(c),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
    subprocess.run([str(DEVKIT/'arm-none-eabi-objcopy'),'--dump-section','.iwram='+str(section),str(ELF)],check=True)
    syms=symbols()
    for mode in (0,1,2,3,4,5,6):run_mode(mode,section.read_bytes(),syms['Blue_BootHandoff'])
    assert 0x03000000<=syms['Blue_BootHandoff']<syms['__sp_irq']
    print('BLUE linked boot handoff passed in ARM/Thumb execution with mocked BIOS, not hardware testing.')
