#!/usr/bin/env python3
"""Execute the production boot snapshot block with modeled SuperCard maps/BIOS copy."""
from pathlib import Path
import struct,subprocess,tempfile
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_ARM,UC_HOOK_INTR,UC_HOOK_MEM_WRITE,UC_HOOK_CODE
from unicorn.arm_const import UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R12
root=Path(__file__).resolve().parents[1];source=(root/'rom_boot.S').read_text()
a=source.index('  @ Preserve the exact installed/chain-loaded kernel');b=source.index('  @ Uncompress/extract the font glyphs',a)
helper=source[source.index('sdram_copy:'):source.index('@ NDS boot codepath')]
with tempfile.TemporaryDirectory() as tmp:
 out=Path(tmp);asm=out/'boot.S'
 asm.write_text('#define GAMEPAK_BASEADDR 0x08000000\n#define FW_MAX_SIZE_KB 2048\n.arm\n.global start\n.equ pogo_return_code,0x08001000\nstart:\n'+source[a:b]+' b done\n'+helper+'\n.pool\ndone: b done\n')
 subprocess.run(['arm-none-eabi-gcc','-mcpu=arm7tdmi','-nostdlib','-Wl,-Ttext=0x03000000','-Wl,-e,start',str(asm),'-o',str(out/'boot.elf')],check=True)
 subprocess.run(['arm-none-eabi-objcopy','-O','binary',str(out/'boot.elf'),str(out/'boot.bin')],check=True)
 symbols={r.split()[-1]:int(r.split()[0],16)for r in subprocess.check_output(['arm-none-eabi-nm',str(out/'boot.elf')],text=True).splitlines()if len(r.split())==3}
 for mode in (4,5):
  for size in (512,521728,1671680,2097152,0,2097664):
   cpu=Uc(UC_ARCH_ARM,UC_MODE_ARM);cpu.mem_map(0x03000000,0x8000);cpu.mem_map(0x08000000,0x2000000)
   cpu.mem_write(0x03000000,(out/'boot.bin').read_bytes())
   fw=bytearray((i*13+mode)&255 for i in range(2097152))
   struct.pack_into('<I',fw,0xcc,size);fw[0xa0:0xa8]=b'SUPERFW\0'
   sdram=bytearray(32*1024*1024)
   if mode==5:sdram[:len(fw)]=fw
   state={'mode':mode,'copies':0,'done':False}
   cpu.mem_write(0x08000000,bytes(fw[:256]));cpu.reg_write(UC_ARM_REG_R12,mode)
   def write(uc,access,address,n,value,user):
    if address==0x09fffffe and value in (4,5):state['mode']=value
   def swi(uc,number,user):
    src,dst,n=(uc.reg_read(r)for r in (UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2));assert n==512
    if 0x08000000<=src<0x0a000000:
     backing=fw if state['mode']==4 else sdram;data=bytes(backing[src-0x08000000:src-0x08000000+n*4])
    else:data=bytes(uc.mem_read(src,n*4))
    assert len(data)==n*4
    if 0x08000000<=dst<0x0a000000:
     assert state['mode']==5;sdram[dst-0x08000000:dst-0x08000000+n*4]=data
    uc.mem_write(dst,data);state['copies']+=1
   def code(uc,address,n,user):
    if address==symbols['done']:state['done']=True;uc.emu_stop()
   cpu.hook_add(UC_HOOK_MEM_WRITE,write);cpu.hook_add(UC_HOOK_INTR,swi);cpu.hook_add(UC_HOOK_CODE,code)
   cpu.emu_start(symbols['start'],0,count=1000000)
   assert state['done'] and state['mode']==mode and cpu.reg_read(UC_ARM_REG_R12)==mode
   if 256<=size<=2097152:
    assert sdram[24*1024*1024:24*1024*1024+size]==fw[:size]
    assert sdram[26*1024*1024:26*1024*1024+2048]==fw[0x1000:0x1800]
    assert state['copies']==2*((size+2047)//2048+1)
   else:assert not state['copies']
print('Production boot snapshot preserves installed/chain-loaded images and return code, rejects bad sizes, and restores source mapping.')
