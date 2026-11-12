#!/bin/sh
set -eu
repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
python3 - "$repo_root" "$tmp/functions.inc" <<'PY'
import re, sys, subprocess
from pathlib import Path
root=Path(sys.argv[1])
functions=[]
for file,names in [('source/ezkernelnew.c',['Launcher_DrawThumbBorderEx']),('source/carousel_motion.h',['Launcher_MotionInset','Launcher_MotionPixel','Launcher_MotionCopyPixels','Launcher_MotionBlitCode'])]:
 text=(root/file).read_text()
 for name in names:
  match=re.search(r'^static [^\n]*\b'+name+r'\([^;{]*\{.*?^}', text, re.M|re.S)
  assert match, name
  functions.append(match.group())
reference=[]
for file,names in [('source/ezkernelnew.c',['Launcher_DrawThumbBorderEx']),('source/carousel_motion.h',['Launcher_MotionInset','Launcher_MotionPixel','Launcher_MotionCopyPixels','Launcher_MotionBlitCode'])]:
 text=subprocess.check_output(['git','-C',str(root),'show','bbcef88:'+file],text=True)
 for name in names:
  match=re.search(r'^static [^\n]*\b'+name+r'\([^;{]*\{.*?^}', text, re.M|re.S)
  assert match, name
  reference.append(match.group())
ref='\n'.join(reference)
for name in ['Launcher_DrawThumbBorderEx','Launcher_MotionInset','Launcher_MotionPixel','Launcher_MotionCopyPixels','Launcher_MotionBlitCode']:
 ref=ref.replace(name, 'Reference_'+name)
functions.append(ref.replace('launcher_art_border_mode','enabled'))
text='\n'.join(functions).replace('__attribute__((noinline, long_call, optimize("O2")))','').replace('__attribute__((noinline, optimize("O3")))','').replace('(u16*)0x06014000','host_map')
Path(sys.argv[2]).write_text(text)
PY
gcc -std=gnu99 -O1 -g -Wall -Wextra -Werror -fsanitize=undefined,bounds \
 -DFUNCTIONS_FILE="\"$tmp/functions.inc\"" "$repo_root/tests/launcher-art-border-host.c" -o "$tmp/test"
"$tmp/test"
