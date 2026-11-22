#!/usr/bin/env python3
"""Actual linked plugin loader/FatFs with FAT16 sectors and a halfword-only cart."""
from pathlib import Path
import argparse,struct,subprocess
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_ARM,UC_HOOK_CODE,UC_HOOK_MEM_WRITE
from unicorn.arm_const import *
root=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--pack',type=Path);p.add_argument('--extension');p.add_argument('--game-bytes',type=int,default=32768);a=p.parse_args();assert 0<a.game_bytes<=3*1024*1024
elf=(root/'firmware.ewram.elf').read_bytes()
version=struct.pack('<I',int(subprocess.check_output(['git','rev-parse','--short=8','HEAD'],cwd=root,text=True).strip(),16))
if (root/'superfw.gba').exists():version=(root/'superfw.gba').read_bytes()[0xc8:0xcc]
syms={r.split()[-1]:int(r.split()[0],16) for r in subprocess.check_output(['arm-none-eabi-nm',str(root/'firmware.ewram.elf')],text=True).splitlines() if len(r.split())==3}
entry=next(v for k,v in syms.items() if k.startswith('load_extemu_rom'))
sms=(root/'emu/smsadvance-v2.5-scptch.gba.bin').read_bytes();pce=(root/'emu/pceadvance-v7.5-scptch.gba.bin').read_bytes()
cases=[('sms','mb',sms),('gg','mb',sms),('pce','mb',pce),('nsf','mbz',bytes([0x10,64,0,0,0x78,0x42,0xf0,0,0xf0,0,0xf0,0,0x60,0])),('txt','bin',struct.pack('<I',0xea00002e)+bytes(252)),('jpg','gba',struct.pack('<I',0xea00002e)+bytes(252))]
if a.pack:
 cases=[(path.stem,path.suffix[1:],path.read_bytes()) for path in sorted(a.pack.iterdir()) if path.suffix in ('.mb','.mbz','.gba','.bin') and path.stat().st_size<0x40000]
if a.extension:cases=[case for case in cases if case[0]==a.extension]
assert cases,'No matching plugin cases'
for extension,suffix,payload in cases:
 plugin_key='sms' if extension=='gg' else extension
 disk=bytearray(8192*512)
 def w16(pos,value):struct.pack_into('<H',disk,pos,value)
 disk[:3]=b'\xeb\x3c\x90';disk[3:11]=b'DSSMM   ';w16(11,512);disk[13]=1;w16(14,1);disk[16]=2;w16(17,512);w16(19,8192);disk[21]=0xf8;w16(22,32);disk[54:62]=b'FAT16   ';w16(510,0xaa55)
 for copy in range(2):w16((1+copy*32)*512,0xfff8);w16((1+copy*32)*512+2,0xffff)
 next_cluster=2
 def allocate(data):
  global next_cluster
  first=next_cluster;count=(len(data)+511)//512
  for i in range(count):
   cluster=next_cluster;next_cluster+=1
   for copy in range(2):w16((1+copy*32)*512+cluster*2,cluster+1 if i+1<count else 0xffff)
   start=(97+cluster-2)*512;disk[start:start+min(512,len(data)-i*512)]=data[i*512:(i+1)*512]
  return first
 def record(name,attr,cluster,size):
  base,_,ext=name.partition('.');return struct.pack('<11sB14xHI',base.upper().encode().ljust(8,b' ')+ext.upper().encode().ljust(3,b' '),attr,cluster,size)
 system=allocate(bytes(512));plug=allocate(bytes(512));game=bytes((i*31)&255 for i in range(a.game_bytes));gc=allocate(game);pc=allocate(payload)
 disk[65*512:65*512+32]=record('SYSTEM',16,system,0);disk[65*512+32:65*512+64]=record('SMOKE.'+extension,32,gc,len(game))
 system_sector=(97+system-2)*512;disk[system_sector:system_sector+32]=record('PLUG',16,plug,0)
 plug_sector=(97+plug-2)*512;disk[plug_sector:plug_sector+32]=record(plugin_key+'.'+suffix,32,pc,len(payload))
 cpu=Uc(UC_ARCH_ARM,UC_MODE_ARM)
 for address,size in [(0x02000000,0x40000),(0x027c0000,0x40000),(0x03000000,0x8000),(0x04000000,0x1000),(0x05000000,0x1000),(0x06000000,0x20000),(0x08000000,0x2000000),(0x01000000,0x1000)]:cpu.mem_map(address,size)
 phoff=struct.unpack_from('<I',elf,28)[0];phsize,n=struct.unpack_from('<HH',elf,42)
 for i in range(n):
  kind,off,address,_,size,_,_,_=struct.unpack_from('<8I',elf,phoff+i*phsize)
  if kind==1 and size:cpu.mem_write(address,elf[off:off+size])
 cpu.mem_write(syms['FatFs'],struct.pack('<I',syms['sdfs']))
 cpu.mem_write(0x098000a0,b'SUPERFW\0');cpu.mem_write(0x098000c8,version+struct.pack('<I',512));cpu.mem_write(0x09a00004,b'DSRT')
 name=('/Smoke.'+extension).encode()+b'\0';cpu.mem_write(0x0203ff00,name)
 cpu.reg_write(UC_ARM_REG_R0,0x0203ff00);cpu.reg_write(UC_ARM_REG_R1,len(game));cpu.reg_write(UC_ARM_REG_R2,syms['pogo_plugin_loader']);cpu.reg_write(UC_ARM_REG_R3,syms['loadpogo_progress']|1)
 cpu.reg_write(UC_ARM_REG_SP,0x03007f00);cpu.reg_write(UC_ARM_REG_LR,0x01000001)
 state={'launched':False,'minimum':0x03007f00,'phases':set()}
 def code(uc,address,size,user):
  state['minimum']=min(state['minimum'],uc.reg_read(UC_ARM_REG_SP))
  if address==syms['launch_pogo_reset']:
   mode,size=(uc.reg_read(r) for r in (UC_ARM_REG_R1,UC_ARM_REG_R2));assert mode==(1 if suffix in ('mb','mbz') else 0)
   patched=bytes(uc.mem_read(0x08000000,size)).count(struct.pack('<II',0xe51ff004,0x09a00000))
   if extension in ('sms','gg','pce'):assert patched,'Known plugin Exit hook missing'
   assert uc.reg_read(UC_ARM_REG_R3)==int(not patched),'Known hooks must preserve the ROM entry; unknown RAM plugins keep the fallback'
   abi=struct.unpack('<4I',uc.mem_read(0x0203fbf8,16));assert abi[0]==len(game) and abi[2:]==(0xfab0babe,2)
   assert uc.mem_read(abi[1],len(game))==game
   state['launched']=True;uc.emu_stop();return
  if address==syms.get('loadrom_progress'):
   state['phases'].add(cpu.mem_read(syms['progress_action'],1)[0])
   assert uc.reg_read(UC_ARM_REG_R0)<=uc.reg_read(UC_ARM_REG_R1)==len(payload)+len(game)
   uc.reg_write(UC_ARM_REG_PC,uc.reg_read(UC_ARM_REG_LR));return
  if address not in (syms['sdcard_read_blocks'],syms['sdcard_write_blocks']):return
  buf,sector,count=(uc.reg_read(r)for r in (UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2));assert count and sector+count<=8192
  if address==syms['sdcard_read_blocks']:uc.mem_write(buf,bytes(disk[sector*512:(sector+count)*512]))
  else:disk[sector*512:(sector+count)*512]=uc.mem_read(buf,count*512)
  uc.reg_write(UC_ARM_REG_R0,0);uc.reg_write(UC_ARM_REG_PC,uc.reg_read(UC_ARM_REG_LR))
 def bus(uc,access,address,size,value,user):
  if 0x08000000<=address<0x0a000000:assert size==2 and address%2==0,('Cartridge write',hex(address),size)
 cpu.hook_add(UC_HOOK_CODE,code);cpu.hook_add(UC_HOOK_MEM_WRITE,bus)
 try:cpu.emu_start(entry|1,0x01000000,count=30000000)
 except Exception:
  print(f'{extension}: PC={cpu.reg_read(UC_ARM_REG_PC):08x} LR={cpu.reg_read(UC_ARM_REG_LR):08x} SP={cpu.reg_read(UC_ARM_REG_SP):08x}')
  raise
 assert state['phases']=={0,1},'Plugin/game progress phases missing'
 assert state['launched'],(extension,suffix,'Native loader returned',cpu.reg_read(UC_ARM_REG_R0))
 assert state['minimum']>=syms['__IWRAM_END__']+128,'Native plugin stack crossed live globals'
 print(f'Native {extension}.{suffix} loader/FatFs/ABI/cart bus passed; stack peak {0x03007f00-state["minimum"]}B.')
