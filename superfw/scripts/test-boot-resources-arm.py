#!/usr/bin/env python3
"""Replay packaged boot resources with the linked, halfword-safe ARM decoder."""
from pathlib import Path
import struct,subprocess
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_ARM,UC_HOOK_MEM_WRITE
from unicorn.arm_const import *
root=Path(__file__).resolve().parents[1]
image=(root/'superfw.gba').read_bytes()
symbols={r.split()[-1]:int(r.split()[0],16) for r in subprocess.check_output(['arm-none-eabi-nm',str(root/'firmware.elf')],text=True).splitlines() if len(r.split())==3}
base=symbols['_start'];boot_end=symbols['_end_bootloader']
assert boot_end-base<=3072 and symbols['_main_end']-base<=1024
extended=b'Chis' in image[0xd0:0xe0]
board='chis' if extended else 'lite' if b'Lite' in image[0xd0:0xe0] else 'sd'
assets={};offset=symbols['assets_start']-base;limit=symbols['assets_end']-base
while offset+8<=limit:
 tag=image[offset:offset+4];size=struct.unpack_from('<I',image,offset+4)[0];offset+=8
 if not size:break
 assert tag not in assets and offset+size<=limit,'Invalid bundled asset directory'
 assets[tag.decode('ascii')]=size;offset+=(size+3)&~3
expected={'sd':{'GCEM'},'lite':{'GCEM'},'chis':{'GCEM','PNES','WASA','NGPG','PCEA','SMSA'}}
assert set(assets)==expected[board],f'{board}: unexpected board-specific media {assets}'
print(f'{board} bundled media: {assets}; unsupported board assets excluded.')

cases=[('fonts','font_start','font_end',root/('res/fonts-ext-dssmm.pack' if extended else 'res/fonts-dssmm.pack'),0x08e80000),
       ('patches','patches_start','patches_end',root/'res/patches.db',0x02000020),
       ('firmware','payload_ewram','payload_ewram_end',root/'firmware.ewram.gba',0x02000020)]
for name,start,end,path,dest in cases:
 raw=path.read_bytes();packed=image[symbols[start]-base:symbols[end]-base]
 if 'upkr_unpack16' not in symbols:
  assert packed[:len(raw)]==raw and len(packed)-len(raw)<4
  print(f'Boot {name}: exact uncompressed resource; no unused boot decoder.')
  continue
 cpu=Uc(UC_ARCH_ARM,UC_MODE_ARM)
 for address,size in ((0x02000000,0x40000),(0x03000000,0x8000),(0x08000000,0x2000000)):cpu.mem_map(address,size)
 cpu.mem_write(0x03000000,image[:boot_end-base])
 source=0x09000000;cpu.mem_write(source,packed)
 cpu.mem_write(dest-32,b'\xa5'*(len(raw)+64))
 # Font replay enters the actual linked boot callsite after its source copy/map.
 # Other resources use the same linked decoder with their real packaged streams.
 if name=='fonts':
  entry=0x03000000+symbols['boot_fonts_unpack']-base
  stop=0x03000000+symbols['boot_fonts_unpack_end']-base
 else:entry=0x03000000+symbols['upkr_unpack16']-base;stop=0x03007ff0
 for reg in (UC_ARM_REG_R4,UC_ARM_REG_R5,UC_ARM_REG_R6,UC_ARM_REG_R7,UC_ARM_REG_R8,UC_ARM_REG_R9,UC_ARM_REG_R10,UC_ARM_REG_R11,UC_ARM_REG_R12):cpu.reg_write(reg,0x12345678)
 for reg,value in ((UC_ARM_REG_R0,dest),(UC_ARM_REG_R1,source),(UC_ARM_REG_SP,0x03007f00),(UC_ARM_REG_LR,stop)):cpu.reg_write(reg,value)
 def write(uc,access,address,size,value,user):
  if dest-32<=address<dest+len(raw)+32:
   assert size==2 and address%2==0,'Boot resource used a byte/word output store'
   assert dest<=address and address+size<=dest+len(raw),'Boot output escaped its allocation'
 cpu.hook_add(UC_HOOK_MEM_WRITE,write)
 cpu.emu_start(entry,stop,count=300000000)
 assert cpu.reg_read(UC_ARM_REG_PC)==stop,'Boot decoder exceeded instruction limit'
 assert cpu.reg_read(UC_ARM_REG_R0)==len(raw)
 assert bytes(cpu.mem_read(dest,len(raw)))==raw,f'{name}: decoded resource differs'
 assert bytes(cpu.mem_read(dest-32,32))==b'\xa5'*32 and bytes(cpu.mem_read(dest+len(raw),32))==b'\xa5'*32
 assert cpu.reg_read(UC_ARM_REG_R12)==0x12345678,'Boot source mapping register clobbered'
 print(f'Boot {name}: {len(packed)} packed -> {len(raw)} exact bytes; ARM halfword writes/guards/source register passed.')
print(f'Boot limits: {boot_end-base}/3072 bytes; stage1={symbols["_main_end"]-base}/1024.')
