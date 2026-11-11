#!/usr/bin/env python3
"""Execute the linked DE boot handoff with mocked cartridge/BIOS, never hardware."""
from pathlib import Path
import argparse,os,re,struct,subprocess
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_ARM,UC_HOOK_CODE,UC_HOOK_INTR,UC_HOOK_MEM_WRITE
from unicorn.arm_const import UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3,UC_ARM_REG_SP,UC_ARM_REG_LR,UC_ARM_REG_PC
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--elf',type=Path,default=root/(root.name+'.elf'))
parser.add_argument('--expect-legacy',action='store_true')
args=parser.parse_args()
devkit=Path(os.environ['DEVKITARM'])/'bin'
symbols={}
for line in subprocess.check_output([str(devkit/'arm-none-eabi-nm'),'-n',str(args.elf)],text=True).splitlines():
 fields=line.split()
 if len(fields)==3:symbols[fields[2]]=int(fields[0],16)
raw=args.elf.read_bytes()
lines=subprocess.check_output([str(devkit/'arm-none-eabi-objdump'),'-h',str(args.elf)],text=True).splitlines()
sections=[]
for index,line in enumerate(lines):
 fields=line.split()
 if len(fields)>=7 and fields[0].isdigit() and all(flag in lines[index+1] for flag in ['CONTENTS','ALLOC','LOAD']):
  size,vma,offset=int(fields[2],16),int(fields[3],16),int(fields[5],16)
  if size:sections.append((vma,raw[offset:offset+size]))
new='DE_BootHandoff' in symbols

def run(mode,key=0,ignore_stack=False):
 uc=Uc(UC_ARCH_ARM,UC_MODE_ARM)
 for base,size in [(0x02000000,0x40000),(0x03000000,0x8000),(0x04000000,0x1000),(0x08000000,0x2000000)]:uc.mem_map(base,size)
 for address,data in sections:uc.mem_write(address,data)
 uc.mem_write(0x02000000,b'\xCC'*0x40000)
 uc.mem_write(symbols['key_L'],struct.pack('<I',key))
 uc.mem_write(symbols['gl_ingame_RTC_open_status'],struct.pack('<H',0x0101))
 uc.mem_write(0x04000208,struct.pack('<H',1))
 payload=b'\xFE\xFF\xFF\xEA\x11\x22\x33\x44'
 rom=(b'\x10\x08\x00\x00\x00'+payload) if mode==4 else payload
 uc.mem_write(0x08000000,rom)
 uc.mem_write(0x0203FBF8,struct.pack('<II',123,0x08000000+len(payload)))
 arguments=b'ARGS-PRESERVED'+bytes(100);uc.mem_write(0x0203FC00,arguments)
 uc.reg_write(UC_ARM_REG_SP,0x0203F000)
 uc.reg_write(UC_ARM_REG_LR,0x0203EF01)
 uc.reg_write(UC_ARM_REG_R0,0x200)
 uc.reg_write(UC_ARM_REG_R1,mode)
 events=[];mapped=False;finished=False
 def code(cpu,address,size,context):
  nonlocal mapped,finished
  if address==symbols['Set_RTC_status']:
   assert cpu.reg_read(UC_ARM_REG_R0)==0x0101
   events.append(('rtc',0x0101));cpu.reg_write(UC_ARM_REG_PC,cpu.reg_read(UC_ARM_REG_LR));return
  if address==symbols['SetRompage']:
   assert cpu.reg_read(UC_ARM_REG_R0)==0x200
   if new:assert int.from_bytes(cpu.mem_read(0x04000208,2),'little')==0
   mapped=True;events.append(('page',0x200));cpu.reg_write(UC_ARM_REG_PC,cpu.reg_read(UC_ARM_REG_LR));return
  if mapped and 0x08000000<=address<0x0A000000:
   raise AssertionError('kernel ROM fetched after game mapping: '+hex(address))
  if address==0x02000000:
   assert mode in (2,4)
   assert cpu.reg_read(UC_ARM_REG_SP)>=0x03000000
   assert bytes(cpu.mem_read(0x02000000,len(payload)))==payload
   events.append(('payload',address));finished=True;cpu.emu_stop()
  assert address!=0x0203EF00,'boot returned through the old EWRAM frame'
 def write(cpu,access,address,size,value,context):
  sp=cpu.reg_read(UC_ARM_REG_SP)
  if not ignore_stack and 0x02000000<=sp<0x0203FBF8 and address<=sp<address+size:
   raise AssertionError('active EWRAM stack overwritten at '+hex(sp))
  if address==0x040000DC and value&0x80000000:
   src,dst=struct.unpack('<II',cpu.mem_read(0x040000D4,8))
   count=(value&0xFFFF)*(4 if value&0x04000000 else 2)
   assert dst+count<=0x0203FBF8
   cpu.mem_write(dst,bytes(cpu.mem_read(src,count)))
 def interrupt(cpu,number,context):
  nonlocal finished
  pc=cpu.reg_read(UC_ARM_REG_PC);opcode=int.from_bytes(cpu.mem_read(pc-2,2),'little')
  assert opcode>>8==0xDF,hex(opcode)
  swi=opcode&255
  if swi==1:
   flags=cpu.reg_read(UC_ARM_REG_R0);assert not flags&2
   if flags&1:
    assert cpu.reg_read(UC_ARM_REG_SP)>=0x03000000
    cpu.mem_write(0x02000000,bytes(0x40000))
   cpu.mem_write(0x04000000,bytes(0x1000));events.append(('reset',flags))
  elif swi==0x11:
   assert mode==4 and cpu.reg_read(UC_ARM_REG_R1)==0x02000000
   assert cpu.reg_read(UC_ARM_REG_SP)>=0x03000000
   cpu.mem_write(0x02000000,payload);events.append(('lz',swi))
  elif swi in (0,0x26):
   events.append(('boot',swi));finished=True;cpu.emu_stop();return
  else:raise AssertionError(('unexpected BIOS',swi))
  for register in [UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_R2,UC_ARM_REG_R3]:cpu.reg_write(register,0xBADC0DE)
 uc.hook_add(UC_HOOK_CODE,code);uc.hook_add(UC_HOOK_MEM_WRITE,write);uc.hook_add(UC_HOOK_INTR,interrupt)
 uc.emu_start(symbols['SetRompageWithHardReset']|1,0,count=1500000)
 assert finished,events
 assert events[:2]==[('rtc',0x0101),('page',0x200)],events
 reset=[event[1] for event in events if event[0]=='reset']
 assert reset==([0xFD] if mode==0 else ([0xFC,0xFC] if mode in (2,4) else [0xFC])),events
 if mode==0:assert bytes(uc.mem_read(0x0203FC00,len(arguments)))==bytes(len(arguments))
 else:assert bytes(uc.mem_read(0x0203FC00,len(arguments)))==arguments
 if mode not in (2,4):assert events[-1]==('boot',0x26 if mode==1 and not key else 0),events
 print('DE linked boot',mode,'key',key,events)

for mode,key in [(0,0),(1,0),(1,1),(2,0),(3,0),(4,0),(5,0)]:
 if args.expect_legacy and mode in (2,3,4):
  try:run(mode,key,ignore_stack=mode==4)
  except AssertionError as error:
   expected='kernel ROM fetched after game mapping' if mode==4 else 'active EWRAM stack overwritten'
   assert expected in str(error),str(error)
   print('Legacy DE mode',mode,'reproduces:',error)
  else:raise AssertionError('legacy failure not reproduced')
 else:run(mode,key)
print('DE linked boot validation passed with mocked BIOS/cartridge, not hardware qualification.')
