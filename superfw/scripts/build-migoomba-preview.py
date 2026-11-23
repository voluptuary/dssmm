#!/usr/bin/env python3
"""Append generated GB/GBC color-stripe cartridges to the pinned generic core."""
from pathlib import Path
import struct
root=Path(__file__).resolve().parents[1];out=root/'build/migoomba-preview';out.mkdir(parents=True,exist_ok=True)
core=(root/'emu/migoomba_v0.1_generic.gba.bin').read_bytes()
logo=bytes.fromhex('CE ED 66 66 CC 0D 00 0B 03 73 00 83 00 0C 00 0D 00 08 11 1F 88 89 00 0E DC CC 6E E6 DD DD D9 99 BB BB 67 63 6E 0E EC CC DD DC 99 9F BB B9 33 3E')
assert len(logo)==48
for color in (False,True):
 rom=bytearray(32768);rom[0x100:0x104]=bytes.fromhex('c3500100');rom[0x104:0x134]=logo;rom[0x134:0x143]=b'DSSMM SMOKE'.ljust(15,b' ');rom[0x143]=0x80 if color else 0;rom[0x147:0x14a]=bytes(3)
 code=bytearray.fromhex('f33100d0afe04021008006083eaa223ecc220520f7afe042e0433ee4e047')
 if color:
  code+=bytes.fromhex('3e80e068')
  for value in struct.pack('<4H',0x7fff,0x001f,0x03e0,0x7c00):code+=bytes([0x3e,value,0xe0,0x69])
 code+=bytes.fromhex('3e91e04018fe');rom[0x150:0x150+len(code)]=code
 checksum=0
 for v in rom[0x134:0x14d]:checksum=(checksum-v-1)&255
 rom[0x14d]=checksum;struct.pack_into('>H',rom,0x14e,(sum(rom[:0x14e])+sum(rom[0x150:]))&65535)
 system='gbc' if color else 'gb';(out/(system+'.rom')).write_bytes(rom);(out/(system+'.gba')).write_bytes(core+rom)
print(out)
