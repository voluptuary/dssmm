#!/usr/bin/env python3
"""Production reset replay; BIOS clears are modeled, card hardware is not."""
from pathlib import Path
import struct
import subprocess
import tempfile
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_INTR
from unicorn.arm_const import (UC_ARM_REG_PC, UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_SP)

root = Path(__file__).resolve().parents[1]
source = (root / 'src/asmutil.S').read_text().split(
    'START_ARM_FUNC(launch_pogo_reset)', 1)[1]
with tempfile.TemporaryDirectory() as temporary:
    work = Path(temporary)
    (work / 'reset.S').write_text('''
.arm
.global launch_pogo_reset
.equ REG_MEMCTRL,0x04000800
.equ REG_IME,0x04000208
launch_pogo_reset:
''' + source)
    subprocess.run(['arm-none-eabi-gcc', '-mcpu=arm7tdmi', '-nostdlib',
                    '-Wl,-Ttext=0x02000000', '-Wl,-e,launch_pogo_reset',
                    str(work / 'reset.S'), '-o', str(work / 'reset.elf')], check=True)
    data = (work / 'reset.elf').read_bytes()
    entry, phoff = struct.unpack_from('<II', data, 24)
    phsize, phnum = struct.unpack_from('<HH', data, 42)
    for fast,mode in ((f,m) for f in (0,1) for m in (0,1,2)):
        cpu = Uc(UC_ARCH_ARM, UC_MODE_ARM)
        for address, size in ((0x02000000, 0x40000), (0x03000000, 0x8000),
                              (0x04000000, 0x1000), (0x08000000, 0x1000)):
            cpu.mem_map(address, size)
            cpu.mem_write(address, bytes([0xa5]) * size)
        for i in range(phnum):
            kind, offset, address, _, size, _, _, _ = struct.unpack_from(
                '<8I', data, phoff + i * phsize)
            if kind == 1 and size:
                cpu.mem_write(address, data[offset:offset + size])
        payload=bytes(range(32))
        cpu.mem_write(0x08000000,payload if mode!=2 else bytes([0x10,32,0,0])+payload)
        abi = bytes(range(96))
        cpu.mem_write(0x0203fbf8, abi)
        cpu.mem_write(0x04000204, struct.pack('<I', 0x4000))
        cpu.reg_write(UC_ARM_REG_R0, fast)
        cpu.reg_write(UC_ARM_REG_R1, mode)
        cpu.reg_write(UC_ARM_REG_R2, len(payload))
        cpu.reg_write(UC_ARM_REG_SP, 0x03007000)
        calls = []

        def interrupt(uc, number, user):
            pc = uc.reg_read(UC_ARM_REG_PC)
            swi = struct.unpack('<I', uc.mem_read(pc - 4, 4))[0] & 0xffffff
            calls.append(swi)
            if swi == 0x010000:
                assert uc.reg_read(UC_ARM_REG_R0) == 0xfe
                uc.mem_write(0x03000000, bytes(0x7e00))
                uc.mem_write(0x04000000, bytes(0x1000))
            elif swi==0x110000:
                assert mode==2
                assert uc.reg_read(UC_ARM_REG_R0)==0x08000000
                assert uc.reg_read(UC_ARM_REG_R1)==0x02000000
                uc.mem_write(0x02000000,payload)
            elif swi == 0:
                expected=(payload+bytes(0x3fbf8-len(payload))) if mode else bytes(0x3fbf8)
                assert uc.mem_read(0x02000000, 0x3fbf8) == expected
                assert uc.mem_read(0x0203fbf8, 96) == abi
                assert struct.unpack('<I', uc.mem_read(0x04000204, 4))[0] == 0x4000
                assert uc.reg_read(UC_ARM_REG_SP) == 0x03007f00
                assert uc.mem_read(0x03007ffa, 1) == bytes([bool(mode)])
                uc.emu_stop()
            else:
                raise AssertionError(hex(swi))

        cpu.hook_add(UC_HOOK_INTR, interrupt)
        cpu.emu_start(entry, 0, count=400000)
        assert calls == ([0x010000,0x110000,0] if mode==2 else [0x010000,0])
print('ARM plugin reset: lower RAM cleared, ABI and WAITCNT preserved at both RAM speeds.')
