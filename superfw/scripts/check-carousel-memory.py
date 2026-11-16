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
hot = [row.split() for row in hot if row.split()[-1:] == ['carousel_hot_pixels']]
assert len(hot) == 1, 'selected image must have a separate EWRAM cache'
address, size, kind, _ = hot[0]
address, size = int(address,16), int(size,16)
assert size == 9600 and kind.lower() == 'b'
assert 0x02000000 <= address < address+size <= 0x03000000, 'hot image outside GBA EWRAM'
print(f'Selected artwork: {size} bytes in GBA EWRAM at {address:#010x}.')

# Cold text uses byte-writable EWRAM, leaving the native16KiB IWRAM stack intact.
symbol_rows=[line.split() for line in subprocess.check_output(
    ['arm-none-eabi-nm','-S',sys.argv[1]],text=True).splitlines()]
for name,expected in [('artwork_rom',256),('shared_values',576),('original',8192)]:
    matches=[row for row in symbol_rows if row[-1:]==[name]]
    assert len(matches)==1 and len(matches[0])==4,f'Missing cold cache {name}'
    address,size,kind,_=matches[0];address,size=int(address,16),int(size,16)
    assert size==expected and kind.lower()=='b'
    assert 0x02000000<=address<address+size<=0x03000000,f'{name} outside GBA EWRAM'
stack=[row for row in symbol_rows if row[-1:]==['__STACK_USR_SIZE__']]
assert len(stack)==1 and int(stack[0][0],16)>=16384,'Native user stack reserve reduced'
print('Cold filename/settings caches are in EWRAM; native16KiB user stack retained.')
