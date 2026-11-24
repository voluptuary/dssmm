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

hot = subprocess.check_output(['arm-none-eabi-nm', '-S', sys.argv[1]], text=True).splitlines()
hot = [row.split() for row in hot if row.split()[-1:] == ['menu_workspace']]
assert len(hot) == 1, 'settings/artwork must share a bounded EWRAM workspace'
address, size, kind, _ = hot[0]
address, size = int(address,16), int(size,16)
assert size == 9600 and address%4==0 and kind.lower() == 'b'
assert 0x02000000 <= address < address+size <= 0x03000000, 'hot image outside GBA EWRAM'
print(f'Shared settings/artwork workspace: {size} bytes in GBA EWRAM at {address:#010x}.')

# Text/settings and sprite records use GBA EWRAM; preserve the native16KiB IWRAM stack.
symbol_rows=[line.split() for line in subprocess.check_output(
    ['arm-none-eabi-nm','-S',sys.argv[1]],text=True).splitlines()]
for name,expected in [('artwork_rom',256),('shared_values',576),('fobjs',512)]:
    matches=[row for row in symbol_rows if row[-1:]==[name]]
    assert len(matches)==1 and len(matches[0])==4,f'Missing cold cache {name}'
    address,size,kind,_=matches[0];address,size=int(address,16),int(size,16)
    assert size==expected and kind.lower()=='b'
    assert 0x02000000<=address<address+size<=0x03000000,f'{name} outside GBA EWRAM'
stack=[row for row in symbol_rows if row[-1:]==['__STACK_USR_SIZE__']]
assert len(stack)==1 and int(stack[0][0],16)>=16384,'Native user stack reserve reduced'
print('Filename/settings caches and sprite records are in EWRAM; native16KiB stack retained.')

assert not any(row[-1:] in (["original"],["carousel_hot_pixels"]) and len(row)==4 and int(row[1],16)>8191 for row in symbol_rows), "Duplicate full settings/artwork buffer retained"
