#!/usr/bin/env python3
"""Replay the real timing fix on owner-supplied, uncompressed plugin files.
Requires Unicorn/devkitARM; never modifies supplied plugins or redistributes them.
"""
from pathlib import Path
import argparse,hashlib,json,struct,subprocess,tempfile
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_ARM,UC_HOOK_MEM_WRITE
from unicorn.arm_const import UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_SP,UC_ARM_REG_LR,UC_ARM_REG_PC
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('plugin_directory',type=Path);args=p.parse_args()
with tempfile.TemporaryDirectory() as temporary:
 work=Path(temporary);elf=work/'patch.elf'
 subprocess.run(['arm-none-eabi-gcc','-O2','-mcpu=arm7tdmi','-mthumb','-ffunction-sections','-fdata-sections','-nostdlib','-Wl,--gc-sections','-Wl,-Ttext=0x03000000','-Wl,-e,pogo_fix_waitcnt',str(root/'src/pogo_plugin.c'),'-o',str(elf)],check=True)
 subprocess.run(['arm-none-eabi-objcopy','-O','binary',str(elf),str(work/'patch.bin')],check=True)
 code=(work/'patch.bin').read_bytes()
 for name in ('sms.mb','pce.mb'):
  raw=(args.plugin_directory/name).read_bytes();block=raw[:8192]
  cpu=Uc(UC_ARCH_ARM,UC_MODE_ARM);cpu.mem_map(0x03000000,0x8000);cpu.mem_map(0x02000000,0x40000)
  cpu.mem_write(0x03000000,code);cpu.mem_write(0x02000000,block)
  for reg,value in [(UC_ARM_REG_R0,0x02000000),(UC_ARM_REG_R1,len(block)),(UC_ARM_REG_SP,0x03007000),(UC_ARM_REG_LR,0x03007f00)]:cpu.reg_write(reg,value)
  cpu.emu_start(0x03000001,0x03007f00,count=100000)
  assert cpu.reg_read(UC_ARM_REG_PC)==0x03007f00 and cpu.reg_read(UC_ARM_REG_R0)==1
  fixed=bytes(cpu.mem_read(0x02000000,len(block)));offsets=[i for i in range(0,len(block),4)if fixed[i:i+4]!=block[i:i+4]]
  assert len(offsets)==1;offset=offsets[0]
  assert struct.unpack_from('<I',block,offset)[0]==0xe3a00014 and struct.unpack_from('<I',fixed,offset)[0]==0xe3a00901
  # Execute the actual startup chain and prove its real hardware write value.
  cpu.mem_map(0x04000000,0x1000);writes=[]
  def write(uc,access,address,size,value,user):
   if address==0x04000204:writes.append((size,value))
  cpu.hook_add(UC_HOOK_MEM_WRITE,write);cpu.emu_start(0x02000000+offset,0x02000000+offset+12,count=3)
  assert writes==[(2,0x4000)]
  print(json.dumps({'plugin':name,'sha256':hashlib.sha256(raw).hexdigest(),'changed_instruction':hex(offset),'waitcnt':hex(writes[0][1]),'scope':'ARM timing fix and startup write, not physical gameplay/exit validation'}))
