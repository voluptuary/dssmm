#!/usr/bin/env python3
"""Verify the linked splash source losslessly decodes to its canonical artwork."""
from pathlib import Path
import re,subprocess,tempfile,os
root=Path(__file__).resolve().parents[1]
source=(root/'source/launcher_splash_lz.h').read_text()
packed=bytes(int(x) for x in re.findall(r'\b\d+\b',source.split('= {',1)[1].split('};',1)[0]))
header=(root/'images/SPLASH.h').read_text()
array=re.search(r'gImage_splash\s*\[76800\][^=]*=\s*\{(.*?)\}',header,re.S)
assert array
raw=bytes(int(x,16) for x in re.findall(r'0x([0-9a-fA-F]{2})',array[1]))
assert len(raw)==76800
with tempfile.TemporaryDirectory() as folder:
 a=Path(folder)/'splash.lz';b=Path(folder)/'splash.raw';a.write_bytes(packed)
 tool=Path(os.environ.get('DEVKITPRO','/opt/devkitpro'))/'tools/bin/gbalzss'
 subprocess.run([str(tool),'d',str(a),str(b)],check=True)
 assert b.read_bytes()==raw
assert 'swi 0x12' in source
print(f'Omega Merkin splash: {len(packed)} packed bytes, exact 76,800-byte artwork match; target VRAM wrapper retained.')
