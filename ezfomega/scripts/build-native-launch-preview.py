#!/usr/bin/env python3
"""Keep SD_list_MENU unchanged; isolate physical/storage side effects for mGBA."""
from pathlib import Path
import argparse,re,subprocess,struct,shutil,json,hashlib
root=Path(__file__).resolve().parents[1];p=argparse.ArgumentParser();p.add_argument('--mode',type=int,choices=(0,1,2),default=0);p.add_argument("--no-alias",action="store_true");p.add_argument("--long-name",action="store_true");a=p.parse_args()
kernel=root;out=root/'build/native-launch';out.mkdir(parents=True,exist_ok=True)
s=(kernel/'source/ezkernelnew.c').read_text()
names=['Process_savefile','init_FAT_table','Check_game_RTS_FAT','Make_recently_play_file','Save_last_launch_mode']
for name in names:
 pattern=r'(^\s*(?:static\s+)?(?:u8|u32|void)\s+)'+name+r'(\s*\([^;{}]*\)\s*\{)'
 s,count=re.subn(pattern,lambda m:m[1]+'fixture_unused_'+name+m[2],s,flags=re.M);assert count==1,(name,count)
prototypes='''
u8 Process_savefile(u32,TCHAR*,u32,BYTE);
void init_FAT_table(void);
u32 Check_game_RTS_FAT(TCHAR*,u32);
void Make_recently_play_file(TCHAR*,TCHAR*);
static void Save_last_launch_mode(u32);
'''
s=s.replace('#include "select_raw.h"','#include "select_raw.h"\n'+prototypes)
s=re.sub(r'#include \"([^\"]+)\"',lambda m:'#include \"'+str((kernel/'source'/m[1]).resolve())+'\"' if (kernel/'source'/m[1]).is_file() else m[0],s)
name=('Native long filename '+('A'*130)+' (USA).gba') if a.long_name else 'Native launch test (USA).gba'
(out/'kernel.c').write_text('#define PREVIEW_TEST_LONG '+str(int(a.long_name))+'\n#define PREVIEW_NO_ALIAS '+str(int(a.no_alias))+'\n#define PREVIEW_TEST_NAME '+json.dumps(name)+'\n'+s)
files=[p.name for directory in ('source','source/ff15')for p in (kernel/directory).glob('*.c')if p.name!='ezkernelnew.c'];files+=['native-launch-preview.c']
wrap='f_opendir f_readdir f_readdir_with_offset f_closedir f_chdir f_getcwd f_stat f_open f_read f_lseek f_close f_mkdir Read_SET_info Read_sav_info Check_mde_file Check_cheat_file Set_64MROM_flag Send_FATbuffer GBApatch_Cleanrom GBApatch_DDR rtc_enable rtc_disenable rtc_gettime SetRompageWithHardReset'.split()
obj=kernel/'build-launch/native-launch-preview.o'
if obj.exists():obj.unlink()
subprocess.run(['make','-j2','BUILD=build-launch','TARGET=native-launch','SOURCES=source source/ff15 tests','CFILES='+' '.join(files),'PREVIEW_SCENE='+str(a.mode),'PREVIEW_LAUNCH_FLAGS='+' '.join('-Wl,--wrap='+x for x in wrap)],cwd=kernel,check=True)
for suffix in ('elf','gba'):shutil.copy2(kernel/('native-launch.'+suffix),out/('native-launch.'+suffix))
rom=out/'native-launch.gba';data=bytearray(rom.read_bytes());assert len(data)<0xff0000;data+=b'\xff'*(0xff0000-len(data));data+=struct.pack('<100sI13sB2x',name.encode()[:99],32768,b'' if a.no_alias else b'NATIVE~1.GBA',int(len(name)>=99));data+=b'\xff'*(0xfff000-len(data));data+=struct.pack('<I',0x20);rom.write_bytes(data)
(out/'fixture.json').write_text(json.dumps({'mode':a.mode,'no_alias':a.no_alias,'name':name,'source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),'source_sha256':hashlib.sha256((kernel/'source/ezkernelnew.c').read_bytes()).hexdigest(),'rom_sha256':hashlib.sha256(data).hexdigest(),'seams':'real launch UI/path resolution/metadata; mocked save, FAT transfer, patch and boot hardware; no physical card qualification'},indent=2)+'\n')
print(rom)
