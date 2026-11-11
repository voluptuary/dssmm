#!/usr/bin/env python3
"""Actual patch writer: staged intersections and target-specific native capacity."""
from pathlib import Path
import argparse,os,re,subprocess,tempfile
root=Path(__file__).resolve().parents[1]
p=root/'targets/blue/source/GBApatch.c'
if not p.exists():p=root/'source/GBApatch.c'
s=p.read_bytes().decode('latin1')
m=re.search(r'^void Write\([^;{}]*?\)\s*\{',s,re.M);assert m
function=s[m.start():s.index('\n}',m.start())+2]
h=(p.parent/'ez_define.h').read_text()
capacity=int(re.search(r'#define PSRAM_CAPACITY_BYTES\s+(0x[0-9A-Fa-f]+)',h)[1],16)
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--case',choices=['banks','last','range','span','huge','null','nor'])
parser.add_argument('--expect-failure',action='store_true')
args=parser.parse_args()
assert not args.expect_failure or args.case
with tempfile.TemporaryDirectory() as folder:
 tmp=Path(folder);(tmp/'patch-write.impl.h').write_text(function)
 subprocess.run([os.environ.get('CC','cc'),'-std=c99','-O1','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-pie','-no-pie','-DPATCH_BANKS='+str(capacity//0x800000),'-I',folder,str(root/'tests/patch-write-host.c'),'-o',str(tmp/'test')],check=True)
 for case in [args.case] if args.case else ['banks','last','range','span','huge','null','nor']:
  r=subprocess.run([str(tmp/'test'),case],capture_output=True,text=True)
  if args.expect_failure:
   assert r.returncode!=0 and ('Assertion' in r.stderr or 'AddressSanitizer' in r.stderr),(case,r.stderr)
   print('Baseline production patch writer reproduces',case,'failure in',p.relative_to(root))
  else:assert r.returncode==0,(case,r.stderr)
print('Patch writer checked for',capacity//0x100000,'MiB target capacity; source',p.relative_to(root))
