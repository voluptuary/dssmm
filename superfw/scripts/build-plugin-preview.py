#!/usr/bin/env python3
"""Run a native emulator or portable plugin with a synthetic ROM in stock mGBA."""
from pathlib import Path
import argparse,ctypes as C,subprocess,struct,tempfile
p=argparse.ArgumentParser();p.add_argument('plugin',type=Path);p.add_argument('--system',choices=['sms','gg','pce'],default='sms');p.add_argument('--out',type=Path,required=True);p.add_argument("--native-pce",action="store_true");a=p.parse_args();assert not a.native_pce or a.system=="pce";a.out.mkdir(parents=True,exist_ok=True)
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as tmp:
 lib=Path(tmp)/'pogo.so';subprocess.run(['gcc','-shared','-fPIC','-O2','-I'+str(root/'src'),str(root/'src/pogo_plugin.c'),'-o',str(lib)],check=True)
 dll=C.CDLL(str(lib));dll.pogo_adapt_image.argtypes=[C.c_void_p,C.c_uint,C.c_uint]
 data=a.plugin.read_bytes();buf=(C.c_uint32*((len(data)+3)//4))();C.memmove(buf,data,len(data));hooks=dll.pogo_adapt_image(buf,len(data),0x08080000)
 (a.out/'plugin.bin').write_bytes(bytes(buf))
 game=bytearray(32768)
 if a.system in ('sms','gg'):
  game[:2]=b'\x18\xfe';game[0x7ff0:0x7ff8]=b'TMR SEGA';game[0x7fff]=0x4c if a.system=='sms' else 0x6c
 else:game[:2]=b'\x80\xfe';game[0x1ffe:0x2000]=b'\x00\xe0'
 (a.out/'game.bin').write_bytes(game)
 filename='Smoke.'+a.system;command=a.plugin.name
 (a.out/'data.S').write_text(f'''.section .rodata
.balign 4
.global plugin_data,plugin_end,game_data
plugin_data:
.incbin "{(a.out/'plugin.bin').resolve()}"
plugin_end:
game_data:
.incbin "{(a.out/'game.bin').resolve()}"
.balign 32768
root:
.word 0xfab0babe,0x80000050
.asciz "{command}"
.space {31-len(command)}
.word plugin_end-plugin_data,plugin_data-root-8
.asciz "{filename}"
.space {31-len(filename)}
.word 32768,game_data-root-8
.section .pogo_exit,"ax"
.arm
.global exit_marker
exit_marker:
 mov r0,#0x04000000
 mov r1,#0
 str r1,[r0,#0x208]
 strh r1,[r0,#0xba]
 strh r1,[r0,#0xc6]
 strh r1,[r0,#0xd2]
 strh r1,[r0,#0xde]
 strh r1,[r0,#0x50]
 str r1,[r0,#0x28]
 str r1,[r0,#0x2c]
 strh r1,[r0,#0x22]
 strh r1,[r0,#0x24]
 mov r1,#0x100
 strh r1,[r0,#0x20]
 strh r1,[r0,#0x26]
 mov r1,#0x400
 orr r1,r1,#3
 strh r1,[r0]
 mov r0,#0x06000000
 mov r1,#0x3e0
 mov r2,#38400
1: strh r1,[r0],#2
 subs r2,r2,#1
 bne 1b
2: b 2b
''')
 (a.out/'main.c').write_text(f'''#include <stdint.h>
extern const uint32_t plugin_data[],plugin_end[],game_data[];
int main(void) {{
 *(volatile uint16_t*)0x04000204=0x4000;
 volatile uint32_t *dest=(volatile uint32_t*)0x02000000;
 for(const uint32_t *src=plugin_data;src<plugin_end;) *dest++=*src++;
 volatile uint32_t *args=(volatile uint32_t*)0x0203fbf8;
 args[0]=32768;args[1]=(uintptr_t)game_data;args[2]=0xfab0babe;args[3]=2;
 const char text[]="/{command}\\0/{filename}";
 for(unsigned i=0;i<sizeof(text);i++)((volatile char*)(args+4))[i]=text[i];
 __asm__ volatile("mov r0,#0xfe; swi 0x010000; ldr r0,=0x03007ffa; mov r1,#1; strb r1,[r0]; swi 0":::"r0","r1","r2","r3","memory");
 for(;;);
}}
''')
 subprocess.run(['arm-none-eabi-gcc','-O2','-marm','-mcpu=arm7tdmi','-specs=gba.specs','-Wl,--section-start=.pogo_exit=0x08080000','-Wl,-u,exit_marker',str(a.out/'main.c'),str(a.out/'data.S'),'-o',str(a.out/'preview.elf')],check=True)
 subprocess.run(['arm-none-eabi-objcopy','--change-section-lma','.pogo_exit=0x08080000','-O','binary',str(a.out/'preview.elf'),str(a.out/'preview.gba')],check=True)
 if a.native_pce:
  # Keep the compiled Exit marker; boot the actual ROM core and native wrapper.
  marker=(a.out/'preview.gba').read_bytes()[0x80000:];assert marker and len(buf)*4+64+len(game)<0x80000
  header=filename.encode().ljust(32,b'\0')+struct.pack('<5I',len(game)+16,4,0,0,0x1a53454e)+b'@          \0'
  assert len(header)==64
  image=bytearray(bytes(buf)+header+game);image+=b'\xff'*(0x80000-len(image));image+=marker
  image[0xb2]=0x96;image[0xbd]=(-sum(image[0xa0:0xbd])-0x19)&255
  (a.out/'preview.gba').write_bytes(image)
 assert (a.out/'preview.gba').stat().st_size<32*1024*1024
 print(a.out/'preview.gba',f'({hooks} exact exit hooks; generated {a.system} ROM, no commercial game)')
