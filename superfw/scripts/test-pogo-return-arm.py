#!/usr/bin/env python3
"""Replay the production cartridge return stub; model only writable cart memory."""
from pathlib import Path
import struct,subprocess,tempfile
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_ARM,UC_HOOK_INTR,UC_HOOK_MEM_WRITE
from unicorn.arm_const import UC_ARM_REG_PC
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as t:
 out=Path(t);source=out/'return.S';source.write_text('.global pogo_return_code\n'+(root/'src/pogo_return.S').read_text())
 subprocess.run(['arm-none-eabi-gcc','-mcpu=arm7tdmi','-nostdlib','-Wl,-Ttext=0x09a00000','-Wl,-e,pogo_return_code',str(source),'-o',str(out/'return.elf')],check=True)
 subprocess.run(['arm-none-eabi-objcopy','-O','binary',str(out/'return.elf'),str(out/'return.bin')],check=True)
 stub=(out/'return.bin').read_bytes()
 for size in (512,521728,1671680,2097152,0,513,2097664):
  cpu=Uc(UC_ARCH_ARM,UC_MODE_ARM)
  cpu.mem_map(0x03000000,0x8000);cpu.mem_map(0x04000000,0x1000);cpu.mem_map(0x08000000,0x2000000)
  image=bytearray((i*37+11)&255 for i in range(min(max(size,512),2097152)))
  struct.pack_into('<I',image,0xcc,size)
  cpu.mem_write(0x09800000,bytes(image));cpu.mem_write(0x09a00000,stub)
  cpu.mem_write(0x04000204,struct.pack('<H',0x14));cpu.mem_write(0x04000208,struct.pack('<I',1))
  cpu.mem_write(0x03007ffa,b'\1');writes=[];done=[]
  def write(uc,access,address,count,value,user):
   if 0x08000000<=address<0x0a000000:
    assert count==2 and not address%2,(hex(address),count)
    if address==0x09fffffe:writes.append(value)
  def interrupt(uc,n,user):
   assert size>=512 and not size%512 and size<=2097152
   assert uc.mem_read(0x08000000,size)==image
   assert uc.mem_read(0x03007ffa,1)==b'\0'
   assert uc.mem_read(0x04000204,2)==struct.pack('<H',0x4000)
   assert uc.mem_read(0x04000208,4)==bytes(4)
   for offset in (0xba,0xc6,0xd2,0xde):assert uc.mem_read(0x04000000+offset,2)==bytes(2)
   assert writes==[0xa55a,0xa55a,5,5,0xa55a,0xa55a,1,1]
   done.append(True);uc.emu_stop()
  cpu.hook_add(UC_HOOK_MEM_WRITE,write);cpu.hook_add(UC_HOOK_INTR,interrupt)
  cpu.emu_start(0x09a00000,0,count=8000000 if 512<=size<=2097152 and not size%512 else 100)
  assert bool(done)==(512<=size<=2097152 and not size%512)
  if not done:assert not writes
 # Scanner scratch is [16,24)MiB; font resources [14.5,16), DB starts28MiB.
 assert 0x09800000>=0x09000000+8*1024*1024
 assert 0x09800000+2097152<=0x09a00000 and 0x09a00000+2048<=0x09c00000
print('Production ARM return restores complete firmware using halfword cart writes; bad sizes rejected, DMA stopped and safe mapping/reset retained.')
