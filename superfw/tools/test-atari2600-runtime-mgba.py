#!/usr/bin/env python3
"""Boot the unmodified runtime Atari core after seeding the Pogoshell arguments."""
from pathlib import Path
import hashlib
import json
import os
import struct
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
out = Path(sys.argv[1] if len(sys.argv) > 1 else root / 'build/atari2600-runtime').resolve()
# This cold-boot seam supplies only the fixed argument block, then branches into
# the runtime image's original entry. It does not compile fixture-only core code.
image = bytearray((out / 'prototype.gba').read_bytes())
entry = struct.unpack_from('<I', image)[0]
assert entry & 0xff000000 == 0xea000000
entry_target = 0x08000008 + ((entry & 0xffffff) << 2)
assert 0x080000c0 <= entry_target < 0x08000000 + len(image)
seed_offset = (len(image) + 3) & ~3
image.extend(bytes(seed_offset - len(image)))
rom_offset = seed_offset + 256
seed = f'''.syntax unified
.arm
.global _start
_start:
 ldr r0,=0x0203fbf8
 ldr r1,=4096
 str r1,[r0]
 ldr r1,={0x08000000 + rom_offset}
 str r1,[r0,#4]
 ldr r1,=0xfab0babe
 str r1,[r0,#8]
 mov r1,#2
 str r1,[r0,#12]
 ldr pc,={entry_target}
.pool
'''
(out / 'runtime-seed.S').write_text(seed)
devkit = Path(os.environ.get('DEVKITARM', '/opt/devkitpro/devkitARM')) / 'bin'
subprocess.run([str(devkit / 'arm-none-eabi-as'), '-mcpu=arm7tdmi',
                str(out / 'runtime-seed.S'), '-o', str(out / 'runtime-seed.o')], check=True)
subprocess.run([str(devkit / 'arm-none-eabi-objcopy'), '-O', 'binary',
                str(out / 'runtime-seed.o'), str(out / 'runtime-seed.bin')], check=True)
code = (out / 'runtime-seed.bin').read_bytes()
assert len(code) <= 256
image.extend(code + bytes(256 - len(code)))
image.extend((out / 'diagnostic.a26').read_bytes())
struct.pack_into('<I', image, 0, 0xea000000 | ((seed_offset - 8) // 4))
fixture = out / 'runtime-fixture'
fixture.mkdir(exist_ok=True)
(fixture / 'prototype.gba').write_bytes(image)
(fixture / 'prototype.json').write_text(json.dumps({
    'purpose': 'fixed-ABI cold-boot seam, actual runtime plugin code',
    'runtime_sha256': hashlib.sha256((out / 'prototype.gba').read_bytes()).hexdigest(),
    'limits': 'Argument seeding substitutes filesystem/card launch; no physical SuperCard emulation.'
}, indent=2) + '\n')
subprocess.run(['bash', str(root / 'tools/test-atari2600-prototype-mgba.sh'), str(fixture)], check=True)
subprocess.run([sys.executable, str(root / 'tools/test-atari2600-menu-mgba.py'), str(fixture)], check=True)
print('Actual Atari runtime: fixed-ABI diagnostic, menu/resume/restart/Exit checks passed.')
