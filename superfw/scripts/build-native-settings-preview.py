#!/usr/bin/env python3
"""Build a ROM running the linked firmware settings/menu with SRAM-backed test SD."""
from pathlib import Path
import argparse,struct,subprocess,os,json,re
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--elf',type=Path,default=root/'firmware.ewram.elf');p.add_argument('--out',type=Path,default=root/'build/native-settings');a=p.parse_args();a.out.mkdir(parents=True,exist_ok=True)
elf=a.elf.read_bytes();nm=subprocess.check_output(['arm-none-eabi-nm',str(a.elf)],text=True)
syms={r.split()[-1]:int(r.split()[0],16) for r in nm.splitlines() if len(r.split())==3}
text='#include <stdint.h>\n'
def blob(name,data):
 global text
 path=a.out/(name+'.bin');path.write_bytes(data)
 subprocess.run(['arm-none-eabi-objcopy','-I','binary','-O','elf32-littlearm','-B','arm','--rename-section','.data=.rodata,alloc,load,readonly,data,contents',str(path),str(a.out/(name+'.o'))],check=True)
 prefix='_binary_'+re.sub(r'[^A-Za-z0-9_]', '_', str(path))
 text+='extern const uint8_t '+prefix+'_start[];\n'
 return prefix+'_start'
segments=[];off=struct.unpack_from('<I',elf,28)[0];size,count=struct.unpack_from('<HH',elf,42)
for i in range(count):
 kind,pos,vaddr,_,filesz,memsz,_,_=struct.unpack_from('<8I',elf,off+i*size)
 if kind==1 and memsz:
  source=blob('native-segment-'+str(i),elf[pos:pos+filesz]) if filesz else '0'
  segments.append((vaddr,filesz,memsz,source))
text+='static const struct {uint32_t address,filesz,memsz;const uint8_t *source;} native_segments[]={\n'
for address,filesz,memsz,source in segments:text+=f'{{0x{address:x},{filesz},{memsz},{source}}},\n'
text+='};\n'
for name in ['FatFs','sdfs','load_settings','f_mkdir','f_stat','sdcard_read_blocks','sdcard_write_blocks','menu_keypress','menu_render','menu_flip','reload_theme','smenu','menu_theme','menu_accent','browser_view','artwork_mode','menu_font','frame_count']:
 text+='#define NATIVE_'+name+f' 0x{syms[name]:08x}u\n'
# Struct offsets are queried from the actual ELF, never from a host ABI.
fields=['menu_tab','uiset.selector','uiset.seloff']
cmd=['arm-none-eabi-gdb','-batch','-ex','file '+str(a.elf)]
for f in fields:cmd+=['-ex','p (char*)&smenu.'+f+'-(char*)&smenu']
values=re.findall(r'\$\d+ = (\d+)',subprocess.check_output(cmd,text=True));assert len(values)==3
for f,v in zip(fields,values):text+='#define OFFSET_'+f.replace('.','_')+' '+v+'\n'
text+='#define TEST_UI_TAB '+('4' if 'keypress_menu_norbrowse' in syms else '3')+'\n'
(a.out/'native-settings-image.h').write_text(text)
objects=[str(x) for x in a.out.glob('native-segment-*.o')]
subprocess.run(['arm-none-eabi-gcc','-Os','-fno-ipa-ra','-fno-builtin','-Wall','-Wextra','-Werror','-mthumb','-mcpu=arm7tdmi','-I',str(a.out),'-specs=gba.specs',str(root/'tests/native-settings-preview.c'),*objects,'-o',str(a.out/'native-settings.elf')],check=True)
subprocess.run(['arm-none-eabi-objcopy','-O','binary',str(a.out/'native-settings.elf'),str(a.out/'native-settings.gba')],check=True)
rom=bytearray((a.out/'native-settings.gba').read_bytes());assert len(rom)<0xe80000
rom+=b'\xff'*(0xe80000-len(rom));font=(root/('res/fonts-ext-dssmm.pack' if 'keypress_menu_norbrowse' in syms else 'res/fonts-dssmm.pack')).read_bytes();rom+=font
# Standard GBA header complement; commercial BIOS is not needed for the harness.
rom[0xb2]=0x96;rom[0xbd]=(-sum(rom[0xa0:0xbd])-0x19)&255
(a.out/'native-settings.gba').write_bytes(rom)
(a.out/'fixture.json').write_text(json.dumps({'native_elf':str(a.elf),'virtual_sd':'64KiB sparse FAT12; low32KiB persists in SRAM','firmware_functions':'actual linked menu/settings/FatFs; only physical SD entrypoints replaced','bios':'not used','limits':'not SuperCard protocol or physical powercycle qualification'},indent=2)+'\n')
print(a.out/'native-settings.gba')
