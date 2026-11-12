#!/usr/bin/env python3
"""Exercise actual DE copy and native/plugin metadata code with filesystem faults."""
from pathlib import Path
import argparse, os, subprocess, tempfile
root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--git-ref')
parser.add_argument('--case', choices=['copy','metadata'], default='copy')
args = parser.parse_args()
source = subprocess.check_output(['git','show',args.git_ref+':source/ezkernelnew.c'],text=True) if args.git_ref else (root/'source/ezkernelnew.c').read_text()
copy = source[source.index('u32 Copy_file('):source.index('u32 Is_bin_file(')]
start = source.index('u32 gamefilesize=0;',source.index('load_file:'))
probe = source[start:source.index('//check',start)]
pre = r"""
static char currentpath[]="/games";
static char pfilename[]="game";
static unsigned char GAMECODE[4];
static int res,error_num,gl_file_overflow;
void init_FAT_table(void) {}
int f_chdir(const char *path) {(void)path;return 0;}
void Show_error_num(int number) {(void)number;}
void Launcher_ShowLaunchError(int number) {(void)number;}
void wait_btn(void) {}
static u32 probe(int is_EMU) {
    (void)is_EMU;
"""
post = r"""
return gamefilesize;
}
int main(void) {
 input_capacity=4096;input=calloc(1,input_capacity);assert(input);
 reset(1);assert(probe(1)==1 && !reads && closes==1);
 reset(8);assert(!probe(0) && closes==1);
 reset(0);assert(!probe(1) && closes==1);
 reset(UINT64_C(0x100000004));assert(!probe(1) && !reads && closes==1);
 memcpy(input+0xAC,"ABCD",4);reset(512);
 assert(probe(0)==512 && reads==1 && !memcmp(GAMECODE,"ABCD",4));
 reset(512);fail_read=1;assert(!probe(0) && closes==1);
 reset(512);short_read=1;assert(!probe(0) && closes==1);
 reset(8);fail_close=1;assert(!probe(1) && closes==1);
 free(input);puts("DE native/plugin metadata: tiny plugin, native header, full-width sizes and I/O faults passed.");
}
"""
with tempfile.TemporaryDirectory() as folder:
 path=Path(folder);(path/'launch-copy.impl.h').write_text(copy)
 harness=(root/'tests/launch-io-host.c').read_text()
 if args.case=='metadata': harness += pre+probe+post
 else: harness += '\nint main(void) {return copy_tests();}\n'
 (path/'test.c').write_text(harness)
 subprocess.run([os.environ.get('CC','cc'),'-std=c99','-O1','-Wall','-Wextra','-Werror','-Wno-unused-variable','-fsanitize=address,undefined','-fno-pie','-no-pie','-I',folder,str(path/'test.c'),'-o',str(path/'test')],check=True)
 subprocess.run([str(path/'test')],check=True)
