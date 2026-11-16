#!/usr/bin/env python3
"""Generate deterministic closed-folder sprites and DSSCC-compatible art packs.
The lettering uses credited DSSMM ASCII source pixels; no host font dependency.
"""
from pathlib import Path
import argparse,struct,zlib,binascii,zipfile
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('output',type=Path,nargs='?',default=root/'build/folder-art');args=p.parse_args()
rows=(root/'assets/fonts/dssmm-ascii-rows.bin').read_bytes()
def rgb(r,g,b):return r|(g<<5)|(b<<10)
outline=rgb(6,6,0);highlight=rgb(31,31,18);top=rgb(31,24,12);bottom=rgb(24,18,6);ink=rgb(6,6,6)
def pixels(width,letter):
 if width==120:
  square=pixels(80,letter)
  return [value for y in range(80) for value in ([0]*20+square[y*80:(y+1)*80]+[0]*20)]
 out=[0]*(width*80);left,right=8,width-8;tab=left+(right-left)//3
 for y in range(14,72):
  for x in range(left,right):
   color=0
   if y<24:
    if x<tab:color=outline if y<16 or x<left+2 or x>=tab-2 else top
   else:
    color=outline if x<left+2 or x>=right-2 or y>=70 or y<26 else highlight if y<29 else top if y<54 else bottom
   out[y*width+x]=color
 glyph=rows[(ord(letter)-32)*12:(ord(letter)-31)*12]
 points=[(x,y) for y,row in enumerate(glyph) for x in range(8) if row&(0x80>>x)]
 x0=min(x for x,y in points);x1=max(x for x,y in points);y0=min(y for x,y in points);y1=max(y for x,y in points)
 scale=min(28//(x1-x0+1),32//(y1-y0+1));assert scale>=3
 gx=(width-(x1-x0+1)*scale)//2;gy=26+(44-(y1-y0+1)*scale)//2
 for x,y in points:
  for dy in range(scale):
   for dx in range(scale):out[(gy+(y-y0)*scale+dy)*width+gx+(x-x0)*scale+dx]=ink
 return out
def bmp(width,data):
 raw=struct.pack('<'+'H'*len(data),*data)
 return struct.pack('<2sI4sI',b'BM',54+len(raw),b'DST1',54)+struct.pack('<IiiHHIIIIII',40,width,-80,1,16,0,len(raw),0,0,0,0)+raw
def png(width,data):
 def chunk(kind,payload):return struct.pack('>I',len(payload))+kind+payload+struct.pack('>I',binascii.crc32(kind+payload)&0xffffffff)
 scan=bytearray()
 for y in range(80):
  scan.append(0)
  for value in data[y*width:(y+1)*width]:scan.extend(((value&31)*255//31,((value>>5)&31)*255//31,((value>>10)&31)*255//31,255 if value else 0))
 return b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',width,80,8,6,0,0,0))+chunk(b'IDAT',zlib.compress(scan,9))+chunk(b'IEND',b'')
args.output.mkdir(parents=True,exist_ok=True)
for style,width in [('IMGS',120),('IMGS2',80)]:
 for letter in '#ABCDEFGHIJKLMNOPQRSTUVWXYZ':
  data=pixels(width,letter);folder=args.output/'SYSTEM'/style/'CUSTOM';folder.mkdir(parents=True,exist_ok=True)
  (folder/f'{letter}.bmp').write_bytes(bmp(width,data))
  folder=args.output/'png'/('title' if width==120 else 'box');folder.mkdir(parents=True,exist_ok=True)
  (folder/f'{letter}.png').write_bytes(png(width,data))
readme='''DSSMM letter-folder artwork

Copy SYSTEM into the root of your SD card, merging folders. The pack contains
# and A through Z: Title120x80 in SYSTEM/IMGS/CUSTOM; Box80x80 in
SYSTEM/IMGS2/CUSTOM. Choose Title or Box artwork in Interface settings.
Existing CUSTOM files with these names are your overrides: keep them if desired.

Matches the complete folder name anywhere on the card. A folder named E uses
E.bmp whether it is at the root or nested inside another folder. Names such as
Europe or E Games do not match E.bmp. No ROM filename/header artwork is replaced.
DSSMM SuperFW588 or newer supports directory CUSTOM art. The pack adds no
firmware/RAM cost. It can also be used by compatible DSSMM CUSTOM folder readers.

BMP files use DSSCC RGB555/DST1 transparency; empty canvas is transparent in the
kernel. PNG versions retain standard alpha transparency for editing/previews.
Letter bitmap credit: DS Style / DSSMM DE ASCII asset, Apache-2.0. Closed-folder
geometry and pack generator: DSSMM contributors, GPL-3.0-or-later. See repository
superfw/docs/FONT-PORT.md and retained component licenses.
'''
(args.output/'README.txt').write_text(readme)
with zipfile.ZipFile(args.output/'dssmm-letter-folder-art.zip','w',zipfile.ZIP_DEFLATED,compresslevel=9) as archive:
 for folder in ('SYSTEM','png'):
  for path in sorted((args.output/folder).rglob('*')):
   if path.is_file():archive.write(path,path.relative_to(args.output))
 archive.write(root/'docs/licenses/dssmm-font-apache-2.0.txt','LICENSES/Apache-2.0.txt')
 archive.write(root/'COPYING','LICENSES/GPL-3.0.txt')
 archive.writestr('README.txt',readme)
print('Generated54 transparent BMPs,54 alpha PNGs and ready-to-copy SYSTEM artwork ZIP.')
