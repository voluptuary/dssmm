#!/usr/bin/env python3
"""Validate the actual marked artwork parser for the active hardware target."""
from pathlib import Path
import argparse, os, re, subprocess, tempfile
root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--git-ref')
args = parser.parse_args()
relative = 'targets/blue/source/ezbluekernel.c' if (root/'targets/blue/source/ezbluekernel.c').exists() else 'source/ezkernelnew.c'
source = subprocess.check_output(['git','-C',str(root),'show',args.git_ref+':'+relative],text=True) if args.git_ref else (root/relative).read_text()
functions = []
for name in ['Launcher_HasTransparentThumbnailMarker','Launcher_TagTransparentThumbnailPixels']:
    match = re.search(r'^static [^;\n]*\b'+name+r'\([^;]*?\n\{', source, re.M)
    assert match, name
    functions.append(source[match.start():source.index('\n}', match.start())+2])
with tempfile.TemporaryDirectory() as folder:
    path = Path(folder)
    (path/'marked-artwork.impl.h').write_text('\n'.join(functions))
    subprocess.run([os.environ.get('CC','cc'),'-std=c99','-O1','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-pie','-no-pie','-I',folder,str(root/'tests/marked-artwork-host.c'),'-o',str(path/'test')],check=True)
    subprocess.run([str(path/'test')],check=True)
