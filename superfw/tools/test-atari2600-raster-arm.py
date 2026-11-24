#!/usr/bin/env python3
"""Check actual raster queue limits and sprite scratch guards on ARM."""
from pathlib import Path
import argparse
import hashlib
import json
import os
import struct
import subprocess
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM
from unicorn.arm_const import UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC

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
                      (0x06000000, 0x20000), (0x01000000, 0x1000)):
    cpu.mem_map(address, size)
cpu.mem_write(0x08000000, (a.out / 'prototype.gba').read_bytes())
cpu.mem_write(0x04000130, b'\xff\x03')
cpu.emu_start(0x08000000, symbols['main'] & ~1, count=1000000)
assert cpu.reg_read(UC_ARM_REG_PC) == symbols['main'] & ~1

def word(name, value):
    cpu.mem_write(symbols[name], struct.pack('<I', value & 0xffffffff))

def read(name):
    return struct.unpack('<I', cpu.mem_read(symbols[name], 4))[0]

def call(name, *args):
    for register, value in zip((UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2), args):
        cpu.reg_write(register, value)
    cpu.reg_write(UC_ARM_REG_SP, 0x03007e00)
    cpu.reg_write(UC_ARM_REG_LR, 0x01000001)
    cpu.emu_start(symbols[name] | 1, 0x01000000, count=1000000)
    assert cpu.reg_read(UC_ARM_REG_PC) == 0x01000000, name
    return cpu.reg_read(UC_ARM_REG_R0)


queue_cases = 0
for function, array, counter, players in (
    ('do_unified_change', 'unified', 'unified_count', 1),
    ('do_plraster_change', 'pl_change', 'pl_change_count', 2),
    ('do_pfraster_change', 'pf_change', 'pf_change_count', 1)):
    for player in range(players):
        for count in (-1, 0, 79, 80, 81, 0x7fffffff):
            cpu.mem_write(symbols[array], bytes([0xcc]) * (players*960 + 16))
            count_address = symbols[counter] + player*4
            cpu.mem_write(count_address, struct.pack('<I', count & 0xffffffff))
            counter_before = bytes(cpu.mem_read(count_address, 4))
            args = (3, 171) if function == 'do_unified_change' else (player, 0, 171)
            call(function, *args)
            actual = bytes(cpu.mem_read(symbols[array], players*960 + 16))
            expected = bytearray(bytes([0xcc]) * (players*960 + 16))
            valid = 0 <= count < 80
            if valid:
                position = player*960 + count*12
                expected[position:position+12] = struct.pack('<3i', 0, 3 if function=='do_unified_change' else 0, 171)
            assert actual == bytes(expected), (function, player, count)
            assert bytes(cpu.mem_read(count_address,4)) == (struct.pack('<I', count+1) if valid else counter_before)
            queue_cases += 1
for function, array, counter, limit in (
    ('do_plraster_change', 'pl_change', 'pl_change_count', 2),
    ('do_pfraster_change', 'pf_change', 'pf_change_count', 1)):
    for invalid in (-1, limit):
        before = bytes(cpu.mem_read(symbols[array], limit*960))
        counts = bytes(cpu.mem_read(symbols[counter], limit*4))
        call(function, invalid & 0xffffffff, 0, 171)
        assert bytes(cpu.mem_read(symbols[array], limit*960)) == before
        assert bytes(cpu.mem_read(symbols[counter], limit*4)) == counts
        queue_cases += 1
sizes = {line.split()[-1]: int(line.split()[1],16)
         for line in subprocess.check_output([str(nm), '-S', str(a.out/'prototype.elf')],text=True).splitlines()
         if len(line.split()) == 4}
assert sizes['colvect'] == 256
# The largest off-screen copies still require valid scratch padding. Track every
# store into the collision vector neighborhood, including reads past the old230.
for position, mode in ((159, 6), (160, 4), (160, 6)):
    cpu.mem_write(symbols['colvect'], bytes(272))
    cpu.mem_write(symbols['pl'], struct.pack('<i8B',position,255,0,0,0,0,mode,0,1))
    word('pl_change_count', 0)
    call('pl_draw1', 0, 0)
    assert bytes(cpu.mem_read(symbols['colvect']+256, 16)) == bytes(16)
    assert any(cpu.mem_read(symbols['colvect'], 256))
record = {'queue_boundary_cases': queue_cases, 'wide_copy_sprite_cases': 3,
          'runtime_sha256': hashlib.sha256((a.out/'prototype.gba').read_bytes()).hexdigest(),
          'limits': 'Pinned core arrays on ARM; overflowed display changes are dropped, not cycle-exact emulation.'}
(a.out/'raster-validation.json').write_text(json.dumps(record,indent=2)+'\n')
print(json.dumps(record))
