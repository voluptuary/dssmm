#!/usr/bin/env python3
"""Retain the original Settings right-panel bevel/stripes in a reusable profile."""
import argparse,re,struct
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');args=p.parse_args()
source=(root/'src/dssmm_background_data.h').read_text()
text='/* Generated from credited DS Style Settings assets; see docs/LAUNCHER-PORT.md. */\nstatic const uint8_t dssmm_panel_profile[2][131][4] = {\n'
for theme in ('light','dark'):
 palette=[int(v,16) for v in re.findall(r'0x[0-9a-f]+',re.search(r'dssmm_'+theme+r'_palette\[\] = \{(.*?)\};',source,re.S)[1])]
 data=struct.unpack('<38400H',(root/f'assets/backgrounds/{theme}-settings.rgb555').read_bytes())
 def pair(x,y):return palette.index(data[y*240+x])|(palette.index(data[y*240+x+1])<<4)
 text+=' {\n'
 for y in range(21,152):
  values=[pair(14,y),palette.index(data[y*240+120]),pair(224,y),pair(226,y)]
  text+='  {'+','.join(f'0x{v:02x}' for v in values)+'},\n'
 text+=' },\n'
text+='};\n'
target=root/'src/res/dssmm-panel.h'
if args.check:assert target.read_text()==text,'Regenerate DS Style panel profile'
else:target.write_text(text)
