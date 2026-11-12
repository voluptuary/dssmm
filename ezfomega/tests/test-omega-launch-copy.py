#!/usr/bin/env python3
"""Actual Omega copy failure handling and launch caller wiring."""
from pathlib import Path
import subprocess,tempfile,os,sys
ROOT=Path(__file__).resolve().parents[1]
source=(ROOT/'source/ezkernelnew.c').read_text()
if len(sys.argv)>1: source=subprocess.check_output(['git','show',sys.argv[1]+':source/ezkernelnew.c'],text=True)
copy=source[source.index('u32 Copy_file('):source.index('u32 Is_bin_file(')]
with tempfile.TemporaryDirectory() as folder:
 c=Path(folder)/'test.c';exe=Path(folder)/'test'
 (Path(folder)/'omega-copy.impl.h').write_text(copy)
 c.write_text((ROOT/'tests/omega/launch-copy.c').read_text())
 subprocess.run([os.environ.get('CC','cc'),'-std=c99','-O1','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-pie','-no-pie','-I',folder,str(c),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
if len(sys.argv)==1:
 import re
 for name in ['LoadEMU2PSRAM','Loadfile2PSRAM']:
  assert re.search(r'res\s*=\s*'+name+r'\(pfilename(?:,is_EMU)?\);\s*if\(res != 0\) \{ Show_error_num\(1\); return 1;',source)
 assert 'make_pogoshell_arguments(plugin + 9' not in source
 print('Omega failed loads request directory rebuild; plugin basename uses its final path component.')
