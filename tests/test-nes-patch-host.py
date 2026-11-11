#!/usr/bin/env python3
"""Production NES detector/probes over native/NOR spans, with legacy repro mode."""
from pathlib import Path
import argparse,os,re,subprocess,tempfile
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--expect-legacy-failures',action='store_true')
args=parser.parse_args()
source_path=root/'targets/blue/source/GBApatch.c'
if not source_path.exists():source_path=root/'source/GBApatch.c'
source=source_path.read_bytes().decode('latin1')
def function(name):
 m=re.search(r'^(?:static|void|bool)\b[^\n;{}]*\b'+name+r'\([^;{}]*?\)\s*\{',source,re.M)
 assert m,name
 return source[m.start():source.index('\n}',m.start())+2]
code=function('CheckNes')+'\n'+function('PatchNes')
bounded=',' in code[:code.index('{')]
if bounded:code=function('NesNativeBytes')+'\n'+code
with tempfile.TemporaryDirectory() as folder:
 p=Path(folder);(p/'nes-patch.impl.h').write_text(code)
 cmd=[os.environ.get('CC','cc'),'-std=c99','-O1','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-pie','-no-pie','-I',folder]
 if bounded:cmd+=['-DBOUNDED_NES_API']
 cmd += [str(root/'tests/nes-patch-host.c'),'-o',str(p/'test')]
 subprocess.run(cmd,check=True)
 cases=['valid','branch','stale','variant','probe','later','short','misaligned','pending']
 if bounded and not args.expect_legacy_failures:cases.append('fuzz')
 for case in cases:
  result=subprocess.run([str(p/'test'),case],capture_output=True,text=True)
  if args.expect_legacy_failures and case!='valid':
   assert result.returncode!=0,(case,'legacy unexpectedly passes')
   assert 'Assertion' in result.stderr or 'AddressSanitizer' in result.stderr,result.stderr
   print(f'Legacy {source_path.relative_to(root)} {case}: reproduced failure')
  else:
   assert result.returncode==0,(case,result.stderr)
 print(f'NES patch checks: {len(cases)} cases, actual {source_path.relative_to(root)}; bounded API={bounded}.')
