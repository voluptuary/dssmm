#!/usr/bin/env python3
"""Compile the real kernel translation unit with a visual-only entry point."""
from pathlib import Path
import argparse,os,subprocess,struct,json,hashlib
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--scene',type=int,default=0,choices=range(9));p.add_argument('--art-dir',type=Path);p.add_argument('--art-style',choices=('title','box'),default='title');a=p.parse_args()
preview=root/'build-preview';preview.mkdir(exist_ok=True)
art='#define PREVIEW_EXTERNAL_ART '+str(int(bool(a.art_dir)))+'\n'
art+='#define PREVIEW_ART_STYLE '+('LAUNCHER_THUMB_STYLE_BOX' if a.art_style=='box' else 'LAUNCHER_THUMB_STYLE_TITLE')+'\n'
if a.art_dir:
 from PIL import Image
 values=[]
 for name in ('moonlit','sunset','skybound'):
  image=Image.open(a.art_dir/(name+'-'+a.art_style+'.png')).convert('RGB')
  assert image.size==((80,80) if a.art_style=='box' else (120,80))
  values.append(','.join(str((r>>3)|((g>>3)<<5)|((b>>3)<<10))for r,g,b in image.getdata()))
 art+='static const u16 preview_artwork_pixels[3]['+str(len(image.getdata()))+']={'+','.join('{'+v+'}'for v in values)+'};\n'
(preview/'preview-artwork.h').write_text(art)
files=[p.name for directory in ('source','source/ff15') for p in (root/directory).glob('*.c') if p.name!='ezkernelnew.c']
files.append('native-menu-preview.c')
(root/'build').mkdir(exist_ok=True)
obj=root/'build-preview/native-menu-preview.o'
if obj.exists():obj.unlink()
subprocess.run(['make','-j2','BUILD=build-preview','TARGET=native-preview','SOURCES=source source/ff15 tests','CFILES='+' '.join(files),'PREVIEW_SCENE='+str(a.scene)],cwd=root,check=True)
import shutil
for suffix in ('elf','gba'):
 shutil.copy2(root/('native-preview.'+suffix),root/'build'/('native-preview.'+suffix))
rom=root/'build/native-preview.gba'
data=bytearray(rom.read_bytes());assert len(data)<0xff0000
data+=b'\xff'*(0xff0000-len(data))
names=('Moonlit Trail (USA).gba','Sunset Circuit (USA).gba','Skybound Courier (USA).gba') if a.art_dir else ('First game (USA).gba','Native menu test ROM (USA).gba','Third game (USA).gba')
for name in names:
 data+=struct.pack('<100sI13sB2x',name.encode(),32768,b'FAKE.GBA',0)
rom.write_bytes(data)
(root/'build/native-preview.json').write_text(json.dumps({'art_style':a.art_style,'external_sample_art':bool(a.art_dir),'scene':a.scene,'source_commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),'source_dirty':bool(subprocess.check_output(['git','status','--porcelain','--','.'],cwd=root,text=True).strip()),'renderer':'actual project kernel translation unit; deterministic clock and filesystem seams','fixture':'ROM-backed read-only PSRAM directory, synthetic decoded artwork cache','limits':'visual tests only; no flash/save/SD protocol qualification','sha256':hashlib.sha256(data).hexdigest()},indent=2)+'\n')
print(rom)
