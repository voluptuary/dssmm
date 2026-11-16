#!/usr/bin/env python3
"""Validate shared screenshot decoding and reject background-only art proofs."""
import binascii,struct,tempfile,unittest,zlib
from pathlib import Path
from png_reader import read_rgb_png,artwork_pixel_count
root=Path(__file__).resolve().parents[1]
def chunk(kind,data):
 return struct.pack('>I',len(data))+kind+data+struct.pack('>I',binascii.crc32(kind+data)&0xffffffff)
class Pixels(unittest.TestCase):
 def test_png_filters(self):
  rows=[bytes((x*37+y*53)%256 for x in range(15)) for y in range(5)]
  raw=bytearray();previous=bytes(15)
  for mode,row in enumerate(rows):
   raw.append(mode)
   for x,value in enumerate(row):
    left=row[x-3] if x>=3 else 0;above=previous[x];upper=previous[x-3] if x>=3 else 0
    estimate=left+above-upper;choices=(left,above,upper)
    paeth=min(enumerate(choices),key=lambda item:(abs(estimate-item[1]),item[0]))[1]
    predict=(0,left,above,(left+above)//2,paeth)[mode]
    raw.append((value-predict)&255)
   previous=row
  png=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',5,5,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(raw))+chunk(b'IEND',b'')
  with tempfile.TemporaryDirectory() as temp:
   p=Path(temp)/'filters.png';p.write_bytes(png);w,h,actual=read_rgb_png(p)
   self.assertEqual((w,h),(5,5));self.assertEqual([bytes(row) for row in actual],rows)
 def test_all_backgrounds_are_not_art(self):
  for path in (root/'assets/backgrounds').glob('*.rgb555'):
   pixels=struct.unpack('<38400H',path.read_bytes());rgb=bytearray()
   for pixel in pixels:
    for shift in (0,5,10):
     channel=(pixel>>shift)&31;rgb.append((channel<<3)|(channel>>2))
   rows=[rgb[y*720:(y+1)*720]for y in range(160)]
   self.assertEqual(artwork_pixel_count(rows),0,path.name)
 def test_fixture_colors_count(self):
  self.assertEqual(artwork_pixel_count([bytes([49,99,0])*1000]),1000)
if __name__=='__main__':unittest.main()
