#!/usr/bin/env python3
"""Center each unchanged 80x80 letter sprite on a 120x80 DST1 canvas."""
import argparse,struct,zipfile,os,tempfile
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('archive',type=Path);p.add_argument('--check',action='store_true');a=p.parse_args()
def title_canvas(data):
    assert data[:2]==b'BM' and struct.unpack_from('<I',data,14)[0]>=40
    width,height,planes,bpp,compression=struct.unpack_from('<iiHHI',data,18)
    offset=struct.unpack_from('<I',data,10)[0]
    assert (width,abs(height),planes,bpp,compression)==(80,80,1,16,0)
    assert len(data)>=offset+12800
    header=bytearray(data[:offset]);header[6:10]=b'DST1'
    body=b''.join(bytes(40)+data[offset+y*160:offset+(y+1)*160]+bytes(40) for y in range(80))
    result=header+body+data[offset+12800:]
    struct.pack_into('<I',result,2,len(result));struct.pack_into('<i',result,18,120);struct.pack_into('<I',result,34,19200)
    return bytes(result)
with zipfile.ZipFile(a.archive) as z:
    entries=[(i,z.read(i.filename)) for i in z.infolist()];lookup=dict((i.filename,d) for i,d in entries)
replacements={f'SYSTEM/IMGS/CUSTOM/{c}.bmp':title_canvas(lookup[f'SYSTEM/IMGS2/CUSTOM/{c}.bmp']) for c in '#ABCDEFGHIJKLMNOPQRSTUVWXYZ'}
if a.check:
    assert all(lookup[k]==v for k,v in replacements.items()),'Title letters must use unscaled centered Box sprites'
else:
    fd,temp=tempfile.mkstemp(dir=a.archive.parent,suffix='.zip');os.close(fd)
    try:
        with zipfile.ZipFile(temp,'w') as z:
            for info,data in entries:z.writestr(info,replacements.get(info.filename,data))
        os.replace(temp,a.archive)
    finally:
        if os.path.exists(temp):os.unlink(temp)
print('27 Title canvases verified; Box sprites and all other archive entries retained.')
