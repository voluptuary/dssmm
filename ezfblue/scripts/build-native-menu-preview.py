#!/usr/bin/env python3
"""Compile the real kernel translation unit with a visual-only entry point."""
from pathlib import Path
import argparse,os,subprocess,struct,json,hashlib
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--scene',type=int,default=0,choices=range(9));a=p.parse_args()
kernel=root/'targets/blue'
files=[p.name for directory in ('source','source/ff15') for p in (kernel/directory).glob('*.c') if p.name!='ezbluekernel.c']
files.append('native-menu-preview.c')
(root/'build').mkdir(exist_ok=True)
obj=kernel/'build-preview/native-menu-preview.o'
if obj.exists():obj.unlink()
subprocess.run(['make','-j2','BUILD=build-preview','TARGET=native-preview','SOURCES=source source/ff15 ../../tests','CFILES='+' '.join(files),'PREVIEW_SCENE='+str(a.scene)],cwd=kernel,check=True)
import shutil
for suffix in ('elf','gba'):
 shutil.copy2(kernel/('native-preview.'+suffix),root/'build'/('native-preview.'+suffix))
rom=root/'build/native-preview.gba'
data=bytearray(rom.read_bytes());assert len(data)<0xff0000
data+=b'\xff'*(0xff0000-len(data))
for name in ('First game (USA).gba','Native menu test ROM (USA).gba','Third game (USA).gba'):
 data+=struct.pack('<100sI13sB2x',name.encode(),32768,b'FAKE.GBA',0)
rom.write_bytes(data)
(root/'build/native-preview.json').write_text(json.dumps({'scene':a.scene,'source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),'source_dirty':bool(subprocess.check_output(['git','status','--porcelain','--','.'],cwd=root,text=True).strip()),'renderer':'actual project kernel translation unit; deterministic clock and filesystem seams','fixture':'ROM-backed read-only PSRAM directory, synthetic decoded artwork cache','limits':'visual tests only; no flash/save/SD protocol qualification','sha256':hashlib.sha256(data).hexdigest()},indent=2)+'\n')
print(rom)
