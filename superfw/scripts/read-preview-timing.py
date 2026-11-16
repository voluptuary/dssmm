#!/usr/bin/env python3
"""Read exact DSSMM debug-counter glyphs from an mGBA screenshot (no OCR package)."""
from pathlib import Path
import argparse,json,sys
root=Path(__file__).resolve().parents[1];sys.path.insert(0,str(root/'tests'))
from png_reader import read_rgb_png
p=argparse.ArgumentParser();p.add_argument('image',type=Path);p.add_argument('--dark',action='store_true');a=p.parse_args()
w,h,rows=read_rgb_png(a.image);assert (w,h)==(240,160)
font=(root/'assets/fonts/dssmm-ascii-rows.bin').read_bytes();ink=(255,255,255)if a.dark else(0,0,0)
result={}
for label,y,prefix in [('background',112,3),('artwork',128,4),('total',144,4)]:
 text=''
 for position in range(11):
  x=96+(prefix+position)*6;bits=[tuple(rows[y+2+dy][(x+dx)*3:(x+dx)*3+3])==ink for dy in range(12)for dx in range(6)]
  matches=[]
  for char in '0123456789 ':
   glyph=font[(ord(char)-32)*12:(ord(char)-31)*12]
   if bits==[bool(glyph[dy]&(128>>dx))for dy in range(12)for dx in range(6)]:matches.append(char)
  assert len(matches)==1,'Use current PREVIEW_TIMING=1/PREVIEW_FONT=1/Light or --dark counter display'
  if matches[0]==' ':break
  text+=matches[0]
 assert text;result[label+'_cycles']=int(text)
assert result['total_cycles']>=result['background_cycles']+result['artwork_cycles']
print(json.dumps({'scope':'mGBA ROM fixture; not physical firmware FPS',**result},indent=2))
