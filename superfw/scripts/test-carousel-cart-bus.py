#!/usr/bin/env python3
"""Run the production carousel cache test as ARM Thumb with a 16-bit cart bus.

Optional diagnostic: requires Python unicorn (tested with 2.0.1) and devkitARM.
This is a CPU/store-width regression, not SuperCard emulation or SD timing.
MENU_SOURCE may point at an older menu.c for a before/after comparison.
"""
import os
from pathlib import Path
import struct
import subprocess
import tempfile
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_THUMB, UC_HOOK_MEM_WRITE, UC_HOOK_CODE
from unicorn.arm_const import UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_R0

root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
    source, elf = Path(tmp)/'cache.c', Path(tmp)/'cache.elf'
    env = dict(os.environ, CAROUSEL_TEST_SOURCE=str(source))
    subprocess.run(['bash', str(root/'scripts/test-carousel-cache.sh')], env=env, check=True)
    subprocess.run(['arm-none-eabi-gcc', '-DTEST_CART_BUS', '-O2', '-mthumb',
                    '-mcpu=arm7tdmi', '-I', str(root/'src'), str(source),
                    str(root/'src/browser_carousel.c'), '-nostartfiles',
                    '-Wl,-Ttext=0x02000000', '-Wl,-e,main', '-specs=nosys.specs',
                    '-o', str(elf)], check=True)
    data = elf.read_bytes()
    assert data[:7] == b'\x7fELF\x01\x01\x01'
    entry, phoff = struct.unpack_from('<II', data, 24)
    phsize, phnum = struct.unpack_from('<HH', data, 42)
    cpu = Uc(UC_ARCH_ARM, UC_MODE_THUMB)
    cpu.mem_map(0x02000000, 0x40000)  # GBA EWRAM
    cpu.mem_map(0x03000000, 0x8000)   # GBA IWRAM/stack
    cpu.mem_map(0x08000000, 0x100000) # cartridge test buffers
    for i in range(phnum):
        kind, offset, address, _, size, _, _, _ = struct.unpack_from('<8I', data, phoff+i*phsize)
        if kind == 1 and size:
            cpu.mem_write(address, data[offset:offset+size])
    symbols = subprocess.check_output(['arm-none-eabi-nm', str(elf)], text=True)
    failures = {int(row.split()[0], 16) for row in symbols.splitlines()
                if len(row.split()) == 3 and row.split()[2] in ('__assert_func', 'abort', '_exit')}
    violations = []
    def write(uc, access, address, size, value, context):
        if 0x08000000 <= address < 0x08100000 and (size not in (2,4) or address % size):
            violations.append(f'unsupported {size}-byte cartridge store at {address:#x}')
            uc.emu_stop()
    def instruction(uc, address, size, context):
        if address in failures:
            violations.append(f'cache assertion/abort at {address:#x}')
            uc.emu_stop()
    cpu.hook_add(UC_HOOK_MEM_WRITE, write)
    cpu.hook_add(UC_HOOK_CODE, instruction)
    stop = 0x03007ff0
    cpu.reg_write(UC_ARM_REG_SP, 0x03007000)
    cpu.reg_write(UC_ARM_REG_LR, stop | 1)
    cpu.emu_start(entry | 1, stop, count=3000000)
    if violations:
        raise SystemExit('; '.join(violations))
    from unicorn.arm_const import UC_ARM_REG_PC
    assert cpu.reg_read(UC_ARM_REG_PC) == stop, 'CPU instruction limit exceeded'
    assert cpu.reg_read(UC_ARM_REG_R0) == 0, 'cache test returned failure'
    print('ARM production-carousel cache assertions pass; no byte/unaligned cartridge stores.')
