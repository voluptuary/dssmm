#!/usr/bin/env python3
"""Bounded plugin policy plus actual linked BIOS wrapper selection, not hardware."""
from pathlib import Path
import os,re,subprocess,tempfile
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as folder:
 exe=Path(folder)/'policy'
 subprocess.run([os.environ.get('CC','cc'),'-std=c99','-O1','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-pie','-no-pie','-I',str(root/'source'),str(root/'tests/omega/plugin-policy.c'),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
objdump=Path(os.environ['DEVKITARM'])/'bin/arm-none-eabi-objdump'
for name,number in [('Omega_DecompressPluginEwram',17),('LZ77UnCompWram',18)]:
 text=subprocess.check_output([str(objdump),'-d','--disassemble='+name,str(root/'omega-port.elf')],text=True)
 assert re.search(r'<'+name+r'>:',text),text
 assert re.search(r'\b(?:svc|swi)\s+(?:0x0*'+format(number,'x')+'|'+str(number)+r')\b',text),text
source=(root/'source/ezkernelnew.c').read_text()
assert 'KernelPluginPayloadValid(PSRAMBase_S98, rom_start_address, 2)' in source
assert 'KernelPluginPayloadValid(PSRAMBase_S98, blockoffset, 4)' in source
card=(root/'source/Ezcard_OP.c').read_text()
assert 'Omega_DecompressPluginEwram((u8*)(0x08000000),(u8*)(0x02000000));' in card
assert 'LZ77UnCompWram((u8*)(0x08000000),(u8*)(0x02000000));' not in card
print('Omega linked plugin WRAM SWI 11h and legacy VRAM SWI 12h remain distinct; payload bounds wired before boot.')
