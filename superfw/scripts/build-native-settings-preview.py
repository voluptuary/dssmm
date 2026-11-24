#!/usr/bin/env python3
"""Build a ROM running the linked firmware settings/menu with SRAM-backed test SD."""
from pathlib import Path
import argparse,struct,subprocess,os,json,re,hashlib
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--elf',type=Path,default=root/'firmware.ewram.elf');p.add_argument('--out',type=Path,default=root/'build/native-settings');p.add_argument('--scene',type=int,choices=range(20),default=0);p.add_argument('--font',type=int,choices=(0,1),default=1);p.add_argument("--reference",action="store_true");p.add_argument("--progress-action",type=int,choices=range(10),default=0);p.add_argument("--recent-title",help="Read-only synthetic recent filename; Start caption test only");p.add_argument('--theme',type=int,choices=(6,7),default=6);p.add_argument('--topbar',type=int,choices=range(6),default=0);p.add_argument("--recent-art",choices=("title","box"));a=p.parse_args();assert not a.recent_art or (a.scene==10 and a.recent_title),"Start art needs scene10 and --recent-title";a.out.mkdir(parents=True,exist_ok=True)
elf=a.elf.read_bytes();nm=subprocess.check_output(['arm-none-eabi-nm',str(a.elf)],text=True)
syms={r.split()[-1]:int(r.split()[0],16) for r in nm.splitlines() if len(r.split())==3}
if a.scene in (4,13) and 'nor_write_progress' not in syms:p.error('NOR scene is unavailable in this linked board image')
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
for name in ['FatFs','sdfs','load_settings','settings_save_flush','settings_save_request','settings_save_pending','settings_save_poll','launcher_page','launcher_selected','f_mkdir','f_stat','sdcard_read_blocks','sdcard_write_blocks','menu_keypress','menu_render','menu_flip','reload_theme','smenu','spop','menu_theme','menu_accent','browser_view','artwork_mode','menu_font','frame_count','framen','menu_topbar','font_base_addr','menu_username','progress_action','loadrom_progress']:
 text+='#define NATIVE_'+name+f' 0x{syms[name]:08x}u\n'
if a.reference:
 for name in ['clock_seconds','clock_frame']:
  text+='#define NATIVE_'+name+f' 0x{syms[name]:08x}u\n'
text+='#define NATIVE_nor_write_progress '+(f'0x{syms["nor_write_progress"]:08x}u' if 'nor_write_progress' in syms else '0')+'\n'
# Struct offsets are queried from the actual ELF, never from a host ABI.
fields=[('smenu',f) for f in ['menu_tab','uiset.selector','uiset.seloff','tools.selector','info.selector']]
if a.recent_title:fields.append(('smenu','recent.maxentries'))
if a.scene>=16:
 assert 'nor_write_progress' in syms,'NOR scenes need a Chis ELF'
 fields += [('smenu',f) for f in ['fbrowser.maxentries','fbrowser.selector','fbrowser.seloff']]
fields += [('spop',f) for f in ['pop_num','submenu','selector','qpop.message','qpop.default_button','qpop.confirm_button','p.load.i.romfn','p.load.i.gcode','p.norld.e','p.update.issfw','p.update.superfw_ver']]
def debug_layout(expression):
 # ptype reads DWARF types even when optimizer location lists cannot materialize
 # an object's address. This GDB build intentionally has no Python interpreter.
 output=subprocess.check_output(['arm-none-eabi-gdb','-batch','-ex','file '+str(a.elf),'-ex','ptype /o '+expression],text=True)
 stack=[];tree=None
 for line in output.splitlines():
  offset=re.search(r'/\*\s*(\d+)\s*\|',line)
  if 'struct {' in line or 'union {' in line:
   stack.append({'offset':int(offset[1]) if offset else (stack[-1]['offset'] if stack else 0),'children':{}})
  elif '}' in line and stack:
   node=stack.pop();name=re.search(r'}\s*(\w+)',line)
   if name and stack:stack[-1]['children'][name[1]]=node
   else:tree=node
  elif offset and stack:
   name=re.search(r'([A-Za-z_]\w*)(?:\[[^]]+\])?\s*;',line)
   if name:stack[-1]['children'][name[1]]={'offset':int(offset[1]),'children':{}}
 assert tree is not None,expression
 result={}
 def visit(node,prefix):
  for name,child in node['children'].items():
   key=prefix+name;result[key]=child['offset'];visit(child,key+'.')
 visit(tree,'');return result
layouts={}
def field_offset(obj,path):
 expression=obj;remaining=path;base=0
 while True:
  if expression not in layouts:layouts[expression]=debug_layout(expression)
  table=layouts[expression]
  if remaining in table:return base+table[remaining]
  prefixes=[key for key in table if remaining.startswith(key+'.')]
  assert prefixes,(expression,remaining)
  prefix=max(prefixes,key=len);base+=table[prefix];expression+='.'+prefix;remaining=remaining[len(prefix)+1:]
for obj,f in fields:text+='#define OFFSET_'+obj+'_'+f.replace('.','_')+' '+str(field_offset(obj,f))+'\n'
# Preserve existing concise Interface field names for the persistence fixture.
for f in ['menu_tab','uiset_selector','uiset_seloff']:text+='#define OFFSET_'+f+' OFFSET_smenu_'+f+'\n'
text+='#define TEST_THEME '+str(a.theme)+'\n#define TEST_TOPBAR '+str(a.topbar)+'\n'
text+='#define TEST_PROGRESS_ACTION '+str(a.progress_action)+'\n'
text+='#define TEST_RECENT '+str(int(bool(a.recent_title)))+'\n'
text+='#define TEST_REFERENCE '+str(int(a.reference))+'\n#define TEST_SCENE '+str(a.scene)+'\n#define TEST_FONT '+str(a.font)+'\n'
text+='#define TEST_UI_TAB '+('4' if 'nor_write_progress' in syms else '3')+'\n'
text+='#define TEST_NOR_CAROUSEL '+str(int(a.scene>=16))+'\n'
def debug_size(expression):
 output=subprocess.check_output(['arm-none-eabi-gdb','-batch','-ex','file '+str(a.elf),'-ex','p sizeof('+expression+')'],text=True)
 return int(re.search(r'= (\d+)',output)[1])
if a.scene>=16:
 text+='#define NATIVE_carousel_meta '+hex(syms['carousel_meta'])+'u\n'
 text+='#define NATIVE_carousel_wrap '+hex(syms['carousel_wrap'])+'u\n'
 meta_size=debug_size('carousel_meta[0]');cache=bytearray(meta_size*5)
 for i in range(5):
  base=i*meta_size
  for field,value,fmt in [('cached',1,'B'),('valid',1,'B'),('index',i,'H'),('mode',2 if a.scene>=18 else 1,'I'),('info.width',80 if a.scene>=18 else 120,'H'),('info.height',80,'H')]:
   struct.pack_into('<'+fmt,cache,base+field_offset('carousel_meta[0]',field),value)
  name=('/NOR/Game '+chr(65+i)+'.gba').encode()+b'\0';off=base+field_offset('carousel_meta[0]','rom');cache[off:off+len(name)]=name
 text+='static const uint16_t native_cache_seed[]={'+','.join(hex(x[0]) for x in struct.iter_unpack('<H',cache))+'};\n'
text+='#define TEST_START_ART '+str(0 if not a.recent_art else 2 if a.recent_art=='box' else 1)+'\n'
if a.recent_art:
 for name in ['artwork_cached','artwork_valid','artwork_cached_mode','artwork_rom','preview_info']:
  text+='#define NATIVE_'+name+' '+hex(syms[name])+'u\n'
 text+='static const char native_recent_path[]='+json.dumps('/Games/'+a.recent_title)+';\n'
(a.out/'native-settings-image.h').write_text(text)
objects=[str(x) for x in a.out.glob('native-segment-*.o')]
subprocess.run(['arm-none-eabi-gcc','-Os','-fno-ipa-ra','-fno-builtin','-Wall','-Wextra','-Werror','-mthumb','-mcpu=arm7tdmi','-I',str(a.out),'-I',str(root/'src'),'-specs=gba.specs',str(root/'tests/native-settings-preview.c'),*objects,'-o',str(a.out/'native-settings.elf')],check=True)
subprocess.run(['arm-none-eabi-objcopy','-O','binary',str(a.out/'native-settings.elf'),str(a.out/'native-settings.gba')],check=True)
rom=bytearray((a.out/'native-settings.gba').read_bytes());assert len(rom)<0xe80000
rom+=b'\xff'*(0xe80000-len(rom))
def insert_record(position,data):
 assert position+len(data)<=len(rom) and rom[position:position+len(data)]==b'\xff'*len(data),'Fixture record overlaps live ROM'
 rom[position:position+len(data)]=data

if a.recent_title:
 recent=('/Games/'+a.recent_title).encode('utf-8')+b'\0'
 max_name=int(re.search(r'^#define MAX_FN_LEN\s+(\d+)',(root/'src/config.h').read_text(),re.M)[1])
 assert len(recent)<=max_name,'Synthetic recent path exceeds bounded record'
 position=field_offset('sdr_state[0]','rentries')
 path_offset=field_offset('sdr_state[0].rentries[0]','fpath')
 record=struct.pack('<HH',0,7)+bytes(path_offset-4)+recent
 insert_record(position,record)
if a.scene>=16:
 position=field_offset('sdr_state[0]','nordata')
 games=field_offset('sdr_state[0]','nordata.games')-position
 game_size=debug_size('sdr_state[0].nordata.games[0]');record=bytearray(games+game_size*5)
 struct.pack_into('<I',record,field_offset('sdr_state[0]','nordata.gamecnt')-position,5)
 for i in range(5):
  base=games+i*game_size;name=('/NOR/Game '+chr(65+i)+'.gba').encode()+b'\0'
  for field,value,fmt in [('gamecode',0x45525042,'I'),('numblks',1,'B'),('bnoffset',5,'B')]:
   struct.pack_into('<'+fmt,record,base+field_offset('sdr_state[0].nordata.games[0]',field),value)
  off=base+field_offset('sdr_state[0].nordata.games[0]','game_name');record[off:off+len(name)]=name
 insert_record(position,record)
 position=field_offset('sdr_state[0]','carousel_art');image_size=debug_size('sdr_state[0].carousel_art[0]')
 width=80 if a.scene>=18 else 120
 for i in range(5):insert_record(position+i*image_size,bytes([70+i*20])*(width*80)+bytes(image_size-width*80))
if a.recent_art:
 insert_record(field_offset('sdr_state[0]','artwork_pixels'),bytes([70])*(80 if a.recent_art=='box' else 120)*80)
font=(root/('res/fonts-ext-dssmm.pack' if 'nor_write_progress' in syms else 'res/fonts-dssmm.pack')).read_bytes();rom+=font
# Standard GBA header complement; commercial BIOS is not needed for the harness.
rom[0xb2]=0x96;rom[0xbd]=(-sum(rom[0xa0:0xbd])-0x19)&255
(a.out/'native-settings.gba').write_bytes(rom)
(a.out/'fixture.json').write_text(json.dumps({'nor_carousel_fixture':a.scene>=16,'progress_action':a.progress_action,'recent_title':a.recent_title,'recent_art':a.recent_art,'scene':a.scene,'font':a.font,'reference':a.reference,'native_elf_sha256':hashlib.sha256(elf).hexdigest(),'native_elf':str(a.elf),'nor_supported':'nor_write_progress' in syms,'virtual_sd':'64KiB sparse FAT12; low32KiB persists in SRAM','firmware_functions':'actual linked menu/settings/FatFs; only physical SD entrypoints replaced','bios':'not used','limits':'not SuperCard protocol or physical powercycle qualification'},indent=2)+'\n')
print(a.out/'native-settings.gba')
