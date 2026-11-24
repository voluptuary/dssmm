#!/usr/bin/env python3
"""Replay linked RIOT countdown, status and WSYNC clock behavior on ARM."""
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

# Explicit boundary values: first decrement, divider, zero, underflow and wrap.
tables = (
    (0, 3, ((0, 3), (1, 2), (3, 0), (4, 255), (5, 254), (259, 0), (260, 255))),
    (3, 3, ((0, 3), (1, 2), (8, 2), (9, 1), (16, 1), (17, 0), (24, 0),
            (25, 255), (26, 254), (280, 0), (281, 255))),
    (6, 2, ((0, 2), (1, 1), (64, 1), (65, 0), (128, 0), (129, 255), (130, 254))),
    (10, 255, ((0, 255), (1, 254), (1024, 254), (1025, 253),
               (260097, 0), (261120, 0), (261121, 255))),
)
cases = 0
for start in (1000, 0xfffffff0):
    for shift, seed, table in tables:
        word('clk', start)
        call('set_timer', shift, seed, 0)
        for elapsed, expected in table:
            word('clk', start + elapsed)
            assert call('do_timer', 0) == expected, (shift, seed, elapsed, expected)
            cases += 1
word('clk', 1000)
call('set_timer', 3, 3, 4)
word('clk', 1004)
assert call('do_timer', 0) == 3 and call('do_timer', 1) == 2
word('clk', 1029)
assert call('do_timer_irq', 0) == 0x80
assert call('do_timer', 0) == 255 and call('do_timer_irq', 0) == 0x80
word('clk', 1030)
assert call('do_timer', 0) == 254 and call('do_timer_irq', 0) == 0
call('atari_timer_reset')
assert call('do_timer', 0) == 0 and call('do_timer_irq', 0) == 0

# Actual decoded WSYNC writes must advance clocks in visible and blanking states.
call('init_hardware')
word('clkcount', 0)
word('ebeamy', 0)
wsync_cases = 0
for state in (1, 2, 8, 16):
    for beam, expected in ((-68, 76), (0, 53), (67, 31), (157, 1), (160, 0)):
        word('clk', 1000)
        word('vbeam_state', state)
        word('ebeamx', beam)
        call('decWrite', 2, 0)
        assert read('clk') == 1000 + expected, (state, beam, read('clk'))
        assert read('ebeamx') == 0xffffffbc
        wsync_cases += 1
# Read the real TIA/RIOT ports, rather than testing key helpers in isolation.
cpu.mem_write(symbols['app_data'] + 12, bytes((1, 1, 0)))  # two joysticks, unswapped
controls = 0
for swap in (0, 1):
    cpu.mem_write(symbols['app_data'] + 14, bytes((swap,)))
    for mask, expected in ((0, 15), (64, 14), (128, 13), (32, 11), (16, 7), (240, 0)):
        cpu.mem_write(0x04000130, struct.pack('<H', 0x3ff ^ mask))
        value = (15 << 4) | expected if swap else (expected << 4) | 15
        assert call('decRead', 0x280) == value, (swap, mask)
        controls += 1
    for fire in (0, 1):
        cpu.mem_write(0x04000130, struct.pack('<H', 0x3ff ^ fire))
        left = 0 if fire and not swap else 128
        right = 0 if fire and swap else 128
        assert call('decRead', 0x0c) == left
        assert call('decRead', 0x0d) == right
        controls += 1
for mask, expected in ((0, 3), (8, 2), (4, 1), (12, 0)):
    cpu.mem_write(0x04000130, struct.pack('<H', 0x3ff ^ mask))
    cpu.mem_write(symbols['riotRead'] + 0x282, b'\x08')
    assert call('decRead', 0x282) == expected | 8
    controls += 1
record = {'timer_boundary_cases': cases, 'wsync_cases': wsync_cases, 'control_port_cases': controls,
          'status': 'bus-offset, underflow flag/read-clear and reset checks passed',
          'runtime_sha256': hashlib.sha256((a.out / 'prototype.gba').read_bytes()).hexdigest(),
          'limits': 'Linked ARM model; legacy instruction-level core is not a cycle-exact TIA/RIOT implementation.'}
(a.out / 'timing-validation.json').write_text(json.dumps(record, indent=2) + '\n')
print(json.dumps(record))
