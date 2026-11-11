#!/usr/bin/env python3
"""Actual OEM-aware custom basename, not a reimplemented parser."""
from pathlib import Path
import os, re, subprocess, tempfile
root=Path(__file__).resolve().parents[1]
source_dir=root/('targets/blue/source' if (root/'targets/blue/source/ezbluekernel.c').exists() else 'source')
source=(source_dir/('ezbluekernel.c' if (source_dir/'ezbluekernel.c').exists() else 'ezkernelnew.c')).read_text()
match=re.search(r'^static void Launcher_CustomThumbFileName\([^;]*?\n\{',source,re.M)
assert match
function=source[match.start():source.index('\n}',match.start())+2]
pre='#include <assert.h>\n#include <string.h>\n#include "fatfs_oem_path.h"\ntypedef unsigned int u32;\n'
post=r"""
int main(void) {
 char result[64];
 Launcher_CustomThumbFileName("/folder/CP936\x81\x5C.gba",result,sizeof result);
 assert(!strcmp(result,"CP936\x81\x5C"));
 Launcher_CustomThumbFileName("C:\\folder\\game.gba",result,sizeof result);
 assert(!strcmp(result,"game"));
 Launcher_CustomThumbFileName("/a/CP936\x81\x5C/next.nes",result,sizeof result);
 assert(!strcmp(result,"next"));
 Launcher_CustomThumbFileName(NULL,result,sizeof result);assert(!result[0]);
 Launcher_CustomThumbFileName("game.gba",result,1);assert(!result[0]);
 assert(FatFsPathLastSeparator("\x81\x5C")==NULL);
 assert(FatFsPathLastSeparator("\x81\x5C\\x")!=NULL);
 assert(FatFsPathLastSeparator("\x81/")!=NULL);
 assert(FatFsPathLastSeparator("\x81")==NULL);
}
"""
with tempfile.TemporaryDirectory() as folder:
 path=Path(folder);(path/'test.c').write_text(pre+function+post)
 subprocess.run([os.environ.get('CC','cc'),'-std=c99','-O1','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-pie','-no-pie','-I',str(source_dir),str(path/'test.c'),'-o',str(path/'test')],check=True)
 subprocess.run([str(path/'test')],check=True)
print('Actual artwork basename preserves CP936 and handles real separators/bounds.')
