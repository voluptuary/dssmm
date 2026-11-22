#!/usr/bin/env python3
"""Check actual ARM icon lookup against C-initialized native tiles in read-only ROM."""
from pathlib import Path
import struct,subprocess,tempfile
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_THUMB,UC_PROT_READ
from unicorn.arm_const import UC_ARM_REG_R0,UC_ARM_REG_SP,UC_ARM_REG_LR,UC_ARM_REG_PC
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
 out=Path(tmp);source=out/'probe.c'
 source.write_text('#include <stdint.h>\n#include "dssmm_background.h"\n#include "res/icons.h"\nvoid *font_base_addr=(void*)0x08e80000;\nuintptr_t main(unsigned size){return (uintptr_t)dssmm_icon_pixels(size);}\n')
 for flavor,flags in [('standard',[]),('extended',['-DFONTS_EXT','-DSUPPORT_NORGAMES'])]:
  elf=out/'probe.elf'
  subprocess.run(['arm-none-eabi-gcc','-O2','-mthumb','-mcpu=arm7tdmi','-DDSSMM_EXTERNAL_BACKGROUNDS',*flags,'-I',str(root/'src'),str(source),str(root/'src/dssmm_background.c'),'-nostartfiles','-Wl,-Ttext=0x02000000','-Wl,-e,main','-specs=nosys.specs','-o',str(elf)],check=True)
  data=elf.read_bytes();entry,phoff=struct.unpack_from('<II',data,24);phsz,phnum=struct.unpack_from('<HH',data,42)
  symbols={r.split()[-1]:int(r.split()[0],16)for r in subprocess.check_output(['arm-none-eabi-nm',str(elf)],text=True).splitlines()if len(r.split())==3}
  native=root/('res/fonts-ext.pack' if flags else 'res/fonts.pack');packed=native.with_name(native.stem+'-dssmm.pack').read_bytes()
  assert packed[:native.stat().st_size]==native.read_bytes(),'Native font prefix changed'
  tag=packed.rfind(b'ICN1');size=struct.unpack_from('<I',packed,tag+4)[0]
  assert size==(4352 if flags else 4096) and tag+8+size==len(packed)
  bg=(native.stat().st_size+3)&~3
  for case in ('valid','bad-bg','bad-icon','bad-size','wrong-request'):
   cpu=Uc(UC_ARCH_ARM,UC_MODE_THUMB)
   for address,length in ((0x02000000,0x40000),(0x03000000,0x8000),(0x01000000,0x1000),(0x08e80000,0x180000)):cpu.mem_map(address,length)
   for i in range(phnum):
    kind,offset,address,_,length,_,_,_=struct.unpack_from('<8I',data,phoff+i*phsz)
    if kind==1 and length:cpu.mem_write(address,data[offset:offset+length])
   blob=bytearray(packed)
   if case=='bad-bg':blob[bg]^=1
   if case=='bad-icon':blob[tag]^=1
   if case=='bad-size':blob[tag+4]^=1
   cpu.mem_write(0x08e80000,bytes(blob));cpu.mem_protect(0x08e80000,0x180000,UC_PROT_READ)
   cpu.reg_write(UC_ARM_REG_R0,size+(case=='wrong-request'));cpu.reg_write(UC_ARM_REG_SP,0x03007f00);cpu.reg_write(UC_ARM_REG_LR,0x01000001)
   cpu.emu_start(entry|1,0x01000000,count=10000);assert cpu.reg_read(UC_ARM_REG_PC)==0x01000000
   result=cpu.reg_read(UC_ARM_REG_R0)
   if case=='valid':
    assert result==0x08e80000+tag+8
    actual=bytes(cpu.mem_read(result,size));expected=bytes(cpu.mem_read(symbols['icons_img'],size))
    assert actual==expected,(flavor,hex(symbols['icons_img']),next((i,a,b)for i,(a,b)in enumerate(zip(actual,expected))if a!=b))
   else:assert not result,(flavor,case,hex(result))
 print('Native board-specific4096/4352 icon bytes match the C tile oracle in read-only standard/extended resources; corrupt tags/sizes and wrong requests rejected.')
