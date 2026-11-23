#!/usr/bin/env python3
"""Replay actual linked scaling against a pixel oracle and halfword-only VRAM."""
from pathlib import Path
import argparse,struct,subprocess
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_THUMB,UC_HOOK_MEM_WRITE,UC_HOOK_CODE
from unicorn.arm_const import UC_ARM_REG_SP,UC_ARM_REG_LR,UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3,UC_ARM_REG_PC
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--elf',type=Path,default=root/'firmware.ewram.elf');p.add_argument("--benchmark",action="store_true");a=p.parse_args()
data=a.elf.read_bytes();assert data[:7]==b'\x7fELF\x01\x01\x01'
phoff=struct.unpack_from('<I',data,28)[0];phsize,phnum=struct.unpack_from('<HH',data,42)
syms={r.split()[2]:int(r.split()[0],16) for r in subprocess.check_output(['arm-none-eabi-nm',str(a.elf)],text=True).splitlines() if len(r.split())==3}
entry=syms['artwork_blit_scaled_cropped'];frame=0x06000010;source=0x08000000;info=0x027ff000;stack=0x03007e00;stop=0x027ffff0
cases=[(39,48,37,37),(40,48,37,37),(39,48,38,37),(-1,15,37,37),(239,159,37,37),(-36,-20,37,37),(0,16,120,80),(0,16,240,144),(1,16,239,80)]
if a.benchmark:cases=[(60,27,120,80)]
peak=0;count=0;instructions=[0]
for width in ((120,) if a.benchmark else (80,120)):
 for transparent in ((False,) if a.benchmark else (False,True)):
  pixels=bytes(0 if transparent and i%7==0 else 20+i%216 for i in range(width*80))
  for x,y,w,h in cases:
   cpu=Uc(UC_ARCH_ARM,UC_MODE_THUMB)
   for address,size in ((0x027c0000,0x40000),(0x03000000,0x8000),(0x06000000,0x20000),(source,0x100000)):cpu.mem_map(address,size)
   for i in range(phnum):
    kind,offset,address,_,size,_,_,_=struct.unpack_from('<8I',data,phoff+i*phsize)
    if kind==1 and size:cpu.mem_write(address,data[offset:offset+size])
   cpu.mem_write(0x06000000,b'\x11'*0x18000);cpu.mem_write(source,pixels);cpu.mem_write(info,struct.pack('<HHBxxx',width,80,transparent))
   cpu.mem_write(stack,struct.pack('<iII',y,w,h))
   violations=[];low=[stack]
   def write(uc,access,address,size,value,context):
    if 0x06000000<=address<0x06020000 and (size!=2 or address&1 or address<frame or address+size>frame+38400):
     violations.append((hex(address),size));uc.emu_stop()
    if 0x03000000<=address<0x03008000:
     low[0]=min(low[0],address)
     if address<0x03004000:violations.append(('stack',hex(address)));uc.emu_stop()
   cpu.hook_add(UC_HOOK_MEM_WRITE,write)
   if a.benchmark:
    def code(uc,address,size,context):instructions[0]+=1
    cpu.hook_add(UC_HOOK_CODE,code)
   for register,value in ((UC_ARM_REG_R0,frame),(UC_ARM_REG_R1,source),(UC_ARM_REG_R2,info),(UC_ARM_REG_R3,x&0xffffffff),(UC_ARM_REG_SP,stack),(UC_ARM_REG_LR,stop|1)):cpu.reg_write(register,value)
   cpu.emu_start(entry|1,stop,count=5_000_000)
   assert not violations,(width,transparent,(x,y,w,h),violations)
   assert cpu.reg_read(UC_ARM_REG_PC)==stop and cpu.reg_read(UC_ARM_REG_R0)==1 and cpu.reg_read(UC_ARM_REG_SP)==stack,(width,transparent,(x,y,w,h),'return')
   expected=bytearray(b'\x11'*38400)
   for fy in range(max(16,y),min(160,y+h)):
    for fx in range(max(0,x),min(240,x+w)):
     value=pixels[((fy-y)*80//h)*width+(fx-x)*width//w]
     if value:expected[fy*240+fx]=value
   actual=cpu.mem_read(frame,38400)
   assert actual==expected,(width,transparent,(x,y,w,h),next((i,actual[i],expected[i]) for i in range(38400) if actual[i]!=expected[i]))
   assert cpu.mem_read(0x06000000,16)==b'\x11'*16 and cpu.mem_read(frame+38400,16)==b'\x11'*16
   peak=max(peak,stack-low[0]);count+=1
print(f'Actual ARM artwork compositor:{count} cases passed; halfword VRAM, untouched edges; stack{peak}B.'+(f' Instructions:{instructions[0]} (not physical cycles).' if a.benchmark else ''))
