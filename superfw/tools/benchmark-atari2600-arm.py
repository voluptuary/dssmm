#!/usr/bin/env python3
"""Count ARM instructions and keypad reads for a fixed generated Atari workload."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import struct
import subprocess
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_CODE, UC_HOOK_MEM_READ
from unicorn.arm_const import UC_ARM_REG_PC

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--out', type=Path, default=Path(__file__).resolve().parents[1] / 'build/atari2600-runtime')
a = p.parse_args()
nm = Path(os.environ.get('DEVKITARM', '/opt/devkitpro/devkitARM')) / 'bin/arm-none-eabi-nm'
symbols = {line.split()[2]: int(line.split()[0], 16)
           for line in subprocess.check_output([str(nm), str(a.out / 'prototype.elf')], text=True).splitlines()
           if len(line.split()) == 3}
cpu = Uc(UC_ARCH_ARM, UC_MODE_ARM)
for address, size in ((0x08000000, 0x2000000), (0x02000000, 0x40000),
                      (0x03000000, 0x8000), (0x04000000, 0x10000),
                      (0x05000000, 0x1000), (0x06000000, 0x20000), (0x07000000, 0x1000)):
    cpu.mem_map(address, size)
cpu.mem_write(0x08000000, (a.out / 'prototype.gba').read_bytes())
cpu.mem_write(0x08800000, (a.out / 'diagnostic.a26').read_bytes())
cpu.mem_write(0x0203fbf8, struct.pack('<4I', 4096, 0x08800000, 0xfab0babe, 2))
cpu.mem_write(0x04000130, b'\xff\x03')
cpu.emu_start(0x08000000, symbols['main'] & ~1, count=1000000)
assert cpu.reg_read(UC_ARM_REG_PC) == symbols['main'] & ~1
counts = {'instructions': 0, 'keypad_reads': 0, 'emulated_instructions': 0}
def instruction(u, address, size, data):
    counts['instructions'] += 1
    if address == symbols['do_screen'] & ~1:
        counts['emulated_instructions'] += 1
        if counts['emulated_instructions'] == 5000:
            u.emu_stop()

def keypad(u, access, address, size, value, data):
    counts['keypad_reads'] += 1

cpu.hook_add(UC_HOOK_CODE, instruction)
cpu.hook_add(UC_HOOK_MEM_READ, keypad, begin=0x04000130, end=0x04000131)
cpu.emu_start(symbols['main'] | 1, 0, count=10000000)
assert counts['emulated_instructions'] == 5000
counts['runtime_sha256'] = hashlib.sha256((a.out / 'prototype.gba').read_bytes()).hexdigest()
counts['limits'] = 'Unicorn instruction counts for fixed generated diagnostic; not GBA cycle timings, FPS or physical performance.'
(a.out / 'benchmark.json').write_text(json.dumps(counts, indent=2) + '\n')
print(json.dumps(counts))
