#!/usr/bin/env python3
"""Replay actual linked ARM settings/FatFs code with a virtual FAT16 SD card."""
from pathlib import Path
import struct,subprocess
from unicorn import *
from unicorn.arm_const import *
import argparse,shutil
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--elf',type=Path,default=root/'firmware.ewram.elf');args=p.parse_args();elf=args.elf.read_bytes()
syms={r.split()[-1]:int(r.split()[0],16) for r in subprocess.check_output([shutil.which('arm-none-eabi-nm'),str(args.elf)],text=True).splitlines() if len(r.split())==3}
u=Uc(UC_ARCH_ARM,UC_MODE_ARM)
for addr,size in [(0x027c0000,0x40000),(0x03000000,0x8000),(0x04000000,0x1000),(0x05000000,0x1000),(0x06000000,0x20000),(0x01000000,0x1000)]:u.mem_map(addr,size)
phoff=struct.unpack_from('<I',elf,28)[0];phsz,phnum=struct.unpack_from('<HH',elf,42)
for i in range(phnum):
 kind,off,vaddr,paddr,filesz,memsz,flags,align=struct.unpack_from('<8I',elf,phoff+i*phsz)
 if kind==1 and filesz:u.mem_write(vaddr,elf[off:off+filesz])
disk=bytearray(8192*512)
def w16(o,v):struct.pack_into('<H',disk,o,v)
def w32(o,v):struct.pack_into('<I',disk,o,v)
disk[:3]=bytes.fromhex('eb3c90');disk[3:11]=b'DSSMM   ';w16(11,512);disk[13]=1;w16(14,1);disk[16]=2;w16(17,512);w16(19,8192);disk[21]=0xf8;w16(22,32);disk[38]=0x29;disk[54:62]=b'FAT16   ';w16(510,0xaa55)
for c in range(2):w16((1+c*32)*512,0xfff8);w16((1+c*32)*512+2,0xffff)
u.mem_write(syms['FatFs'],struct.pack('<I',syms['sdfs']))
minimum=0x03007f00;accesses=[];steps=0
rds,wrs=syms['sdcard_read_blocks'],syms['sdcard_write_blocks']
def step(uc,addr,size,user):
 global minimum,steps
 steps+=1;minimum=min(minimum,uc.reg_read(UC_ARM_REG_SP))
 if addr not in (rds,wrs):return
 buf=uc.reg_read(UC_ARM_REG_R0);sec=uc.reg_read(UC_ARM_REG_R1);cnt=uc.reg_read(UC_ARM_REG_R2)
 assert cnt and sec+cnt<=8192,(hex(addr),hex(buf),sec,cnt)
 if addr==rds:uc.mem_write(buf,bytes(disk[sec*512:(sec+cnt)*512]))
 else:disk[sec*512:(sec+cnt)*512]=uc.mem_read(buf,cnt*512);accesses.append((hex(buf),cnt,buf%4))
 uc.reg_write(UC_ARM_REG_R0,0);uc.reg_write(UC_ARM_REG_PC,uc.reg_read(UC_ARM_REG_LR))
u.hook_add(UC_HOOK_CODE,step)
def invalid(uc,access,address,size,value,user):
 print('INVALID',access,hex(address),size,'PC',hex(uc.reg_read(UC_ARM_REG_PC)),'SP',hex(uc.reg_read(UC_ARM_REG_SP)));return False
u.hook_add(UC_HOOK_MEM_INVALID,invalid)
def call(name,*args):
 for r,v in zip([UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3],args):u.reg_write(r,v)
 u.reg_write(UC_ARM_REG_SP,0x03007f00);u.reg_write(UC_ARM_REG_LR,0x01000001)
 u.emu_start(syms[name]|1,0x01000000,count=15000000)
 assert u.reg_read(UC_ARM_REG_PC)==0x01000000,(name,'instruction budget',hex(u.reg_read(UC_ARM_REG_PC)))
 return u.reg_read(UC_ARM_REG_R0)
call('load_settings')
save=next(name for name in syms if name.startswith('save_settings'))
keys=['menu_theme','menu_accent','browser_view','artwork_mode']
# Later saves exercise canonical-value preservation with a loaded baseline.
for values in [(7,3,2,2),(6,4,1,1),(7,1,0,0),(3,0,0,2)]:
 for key,value in zip(keys,values):u.mem_write(syms[key],bytes([value]))
 before=len(accesses)
 assert call(save)==1
 assert len(accesses)>before,'Changed preferences were not written'
 unchanged=len(accesses)
 assert call(save)==1 and len(accesses)==unchanged,'Unchanged exit rewrote SD settings'
 for key in keys:u.mem_write(syms[key],b'\0')
 call('load_settings')
 for key,value in zip(keys,values):
  assert u.mem_read(syms[key],1)==bytes([value]),(key,value)
assert minimum>=syms['__IWRAM_END__']+128,'Native save stack crossed live data'
print(f'Actual ARM ELF/FatFs settings save/reload passed; stack peak{0x03007f00-minimum}B, {len(accesses)} virtual SD writes (not physical hardware).')
