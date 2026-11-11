#!/usr/bin/env python3
"""Actual target save loader with bank/range canaries and SD fault injection."""
from pathlib import Path
import argparse, os, re, subprocess, tempfile
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--git-ref')
parser.add_argument('--case',choices=['size','faults'],default='size')
args=parser.parse_args()
relative='targets/blue/source/ezbluekernel.c' if (root/'targets/blue/source/ezbluekernel.c').exists() else 'source/ezkernelnew.c'
s=subprocess.check_output(['git','-C',str(root),'show',args.git_ref+':'+relative],text=True) if args.git_ref else (root/relative).read_text()
m=re.search(r'^u32 (?:IWRAM_CODE )?Loadsavefile\([^;]*?\n\{',s,re.M);assert m
function=s[m.start():s.index('\n}',m.start())+2]
with tempfile.TemporaryDirectory() as folder:
 p=Path(folder);(p/'save-load.impl.h').write_text(function)
 subprocess.run([os.environ.get('CC','cc'),'-std=c99','-O1','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-pie','-no-pie','-I',folder,str(root/'tests/save-load-host.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test'),args.case],check=True)
