#!/usr/bin/env python3
"""Actual ARM background renderer with GBA halfword VRAM writes (Unicorn)."""
from pathlib import Path
import struct,subprocess,tempfile
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_THUMB,UC_HOOK_MEM_WRITE
from unicorn.arm_const import UC_ARM_REG_SP,UC_ARM_REG_LR,UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_PC
root=Path(__file__).resolve().parents[1]
subprocess.run(['python3',str(root/'tools/pack-dssmm-resources.py')],check=True)
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'main.c').write_text('''#include "dssmm_background.h"
void *font_base_addr=(void *)0x08e80000;
int main(unsigned dark,unsigned start) {
 dssmm_background_palette((volatile uint16_t *)0x05000000,dark);
 dssmm_background_draw((volatile uint8_t *)0x06000000,dark,start);
 return 0;
}''')
 for variant,flags in [('embedded',[]),('standard',['-DDSSMM_EXTERNAL_BACKGROUNDS']),('extended',['-DDSSMM_EXTERNAL_BACKGROUNDS','-DFONTS_EXT']),('missing',['-DDSSMM_EXTERNAL_BACKGROUNDS'])]:
  elf=p/'test.elf'
  subprocess.run(['arm-none-eabi-gcc','-O2','-mthumb','-mcpu=arm7tdmi','-I',str(root/'src'),str(p/'main.c'),str(root/'src/dssmm_background.c'),*flags,'-nostartfiles','-Wl,-Ttext=0x02000000','-Wl,-e,main','-specs=nosys.specs','-o',str(elf)],check=True)
  data=elf.read_bytes();entry,phoff=struct.unpack_from('<II',data,24);phsize,phnum=struct.unpack_from('<HH',data,42)
  for dark in (0,1):
   for start in range(6):
    cpu=Uc(UC_ARCH_ARM,UC_MODE_THUMB)
    for address,size in ((0x02000000,0x40000),(0x03000000,0x8000),(0x05000000,0x1000),(0x06000000,0x18000),(0x08e80000,0x180000)):cpu.mem_map(address,size)
    for i in range(phnum):
     kind,off,addr,_,size,_,_,_=struct.unpack_from('<8I',data,phoff+i*phsize)
     if kind==1 and size:cpu.mem_write(addr,data[off:off+size])
    if variant!='embedded':cpu.mem_write(0x08e80000,(root/'res'/('fonts-ext-dssmm.pack' if variant=='extended' else 'fonts-dssmm.pack')).read_bytes())
    if variant=='missing':cpu.mem_write(0x08e80000+len((root/'res/fonts.pack').read_bytes()),bytes(4))
    def store(uc,access,addr,size,value,context):
     if 0x08e80000<=addr<0x09000000:raise AssertionError('Background modified native read-only font partition')
     if 0x05000000<=addr<0x06018000:assert size in (2,4) and addr%size==0,f'Unsafe video write {addr:#x}/{size}'
    cpu.hook_add(UC_HOOK_MEM_WRITE,store)
    stop=0x03007ff0;cpu.reg_write(UC_ARM_REG_SP,0x03007000);cpu.reg_write(UC_ARM_REG_LR,stop|1);cpu.reg_write(UC_ARM_REG_R0,dark);cpu.reg_write(UC_ARM_REG_R1,start)
    cpu.emu_start(entry|1,stop,count=2000000);assert cpu.reg_read(UC_ARM_REG_PC)==stop,'Renderer instruction limit'
    pal=struct.unpack('<16H',cpu.mem_read(0x05000000,32));frame=cpu.mem_read(0x06000000,38400)
    actual=struct.pack('<38400H',*(pal[x] for x in frame))
    if start and variant!='missing':expected=(root/'assets/backgrounds'/f"{'dark' if dark else 'light'}-{('start','settings','horizontal','vertical','grid')[start-1]}.rgb555").read_bytes()
    else:expected=b''.join(struct.pack('<H',((0 if y&1 else 0x0c63) if dark else (0x7fff if y&1 else 0x739c)))*240 for y in range(160))
    assert actual==expected,'ARM output differs from native DE background'
 print('ARM embedded/SD/Lite/Chis asset renderers match DE pixels with read-only font assets and halfword video writes.')
