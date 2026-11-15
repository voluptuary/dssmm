#!/usr/bin/env python3
"""Require carousel metadata in byte-writable GBA RAM in the actual firmware."""
import subprocess
import sys

rows = subprocess.check_output(['arm-none-eabi-nm', '-S', sys.argv[1]], text=True).splitlines()
rows = [row.split() for row in rows if row.split()[-1:] == ['carousel_meta']]
assert len(rows) == 1, 'carousel metadata must be a separate internal-RAM object'
address, size, kind, _ = rows[0]
address, size = int(address, 16), int(size, 16)
assert kind.lower() in ('b', 'd'), 'carousel metadata must be writable'
# EWRAM mirrors are used by the native linker; IWRAM is the normal .bss home.
assert (0x02000000 <= address < address+size <= 0x03000000 or
        0x03000000 <= address < address+size <= 0x03008000), 'metadata on cartridge bus'
print(f'Carousel metadata: {size} bytes in internal GBA RAM at {address:#010x}.')
