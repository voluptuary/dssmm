#!/usr/bin/env python3
"""Verify packed IGM bytes/metadata in the actual ELF; optional ARM replay."""
from pathlib import Path
import argparse,hashlib,struct,subprocess,tempfile
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser();parser.add_argument('--arm',action='store_true');args=parser.parse_args()
raw=(root/'ingamemenu.payload').read_bytes();packed=(root/'ingamemenu.payload.comp').read_bytes()
assert len(raw)%4==0
runtime=struct.unpack_from('<I',raw,64)[0]
assert runtime==len(raw)+162*1024
with tempfile.TemporaryDirectory() as temporary:
    work=Path(temporary);decoded=work/'decoded'
    subprocess.run([str(root/'upkr.elf'),'-d',str(root/'ingamemenu.payload.comp'),str(decoded)],check=True,capture_output=True)
    assert decoded.read_bytes()==raw,'UPKR host roundtrip changed IGM'
    elf=(root/'firmware.ewram.elf').read_bytes()
    phoff=struct.unpack_from('<I',elf,28)[0];phsize,phnum=struct.unpack_from('<HH',elf,42)
    def elf_bytes(address,count):
        for i in range(phnum):
            kind,offset,base,_,size,_,_,_=struct.unpack_from('<8I',elf,phoff+i*phsize)
            if kind==1 and base<=address and address+count<=base+size:
                return elf[offset+address-base:offset+address-base+count]
        raise AssertionError(f'ELF data missing at {address:#x}')
    symbols={row.split()[-1]:int(row.split()[0],16) for row in subprocess.check_output(
        ['arm-none-eabi-nm',str(root/'firmware.ewram.elf')],text=True).splitlines() if len(row.split())==3}
    assert elf_bytes(symbols['ingame_menu_packed'],len(packed))==packed
    assert struct.unpack('<I',elf_bytes(symbols['ingame_menu_packed_size'],4))[0]==len(packed)
    assert struct.unpack('<I',elf_bytes(symbols['ingame_menu_runtime_size'],4))[0]==runtime
    if args.arm:
        from unicorn import Uc,UC_ARCH_ARM,UC_MODE_ARM,UC_HOOK_MEM_WRITE
        from unicorn.arm_const import UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_SP,UC_ARM_REG_LR,UC_ARM_REG_PC
        subprocess.run(['arm-none-eabi-gcc','-mcpu=arm7tdmi','-nostdlib','-Wl,-Ttext=0x02000000','-Wl,-e,upkr_unpack16',
                        str(root/'src/upkrunpack16.S'),'-o',str(work/'unpack.elf')],check=True)
        subprocess.run(['arm-none-eabi-objcopy','-O','binary',str(work/'unpack.elf'),str(work/'unpack.bin')],check=True)
        cpu=Uc(UC_ARCH_ARM,UC_MODE_ARM)
        for address,size in ((0x02000000,0x40000),(0x03000000,0x8000),(0x08000000,0x100000)):
            cpu.mem_map(address,size)
        code=(work/'unpack.bin').read_bytes();cpu.mem_write(0x02000000,code)
        source=0x02010000;dest=0x08000020;stop=0x03007ff0
        cpu.mem_write(source,packed);cpu.mem_write(dest-32,bytes([0xa5])*(len(raw)+64))
        def write(uc,access,address,size,value,user):
            if 0x08000000<=address<0x08100000:
                assert size==2 and address%2==0,'Unsafe cartridge write'
                assert dest<=address<dest+len(raw),'IGM output escaped bounds'
        cpu.hook_add(UC_HOOK_MEM_WRITE,write)
        for reg,value in ((UC_ARM_REG_R0,dest),(UC_ARM_REG_R1,source),(UC_ARM_REG_SP,0x03007000),(UC_ARM_REG_LR,stop)):
            cpu.reg_write(reg,value)
        cpu.emu_start(0x02000000,stop,count=15000000)
        assert cpu.reg_read(UC_ARM_REG_PC)==stop,'ARM unpacker instruction limit'
        assert cpu.reg_read(UC_ARM_REG_R0)==len(raw)
        assert cpu.mem_read(dest,len(raw))==raw
        assert cpu.mem_read(dest-32,32)==bytes([0xa5])*32
        assert cpu.mem_read(dest+len(raw),32)==bytes([0xa5])*32
print(f'IGM package: {len(raw)} -> {len(packed)} bytes; runtime={runtime}; exact ELF/host'+('/ARM halfword replay' if args.arm else '')+' verified.')
