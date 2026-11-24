#!/usr/bin/env python3
"""Replay experimental startup and cartridge validation from the linked ARM image."""
from pathlib import Path
import argparse
import json
import os
import struct
import subprocess
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_MEM_WRITE
from unicorn.arm_const import UC_ARM_REG_R0, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC

root = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser(description=__doc__)
p.add_argument("--out", type=Path, default=root / "build/atari2600-runtime")
a = p.parse_args()
nm = Path(os.environ.get("DEVKITARM", "/opt/devkitpro/devkitARM")) / "bin/arm-none-eabi-nm"
symbols = {line.split()[2]: int(line.split()[0], 16)
           for line in subprocess.check_output([str(nm), str(a.out / "prototype.elf")], text=True).splitlines()
           if len(line.split()) == 3}
assert "atari_diagnostic_rom" not in symbols, "Runtime must not embed the test ROM"
cpu = Uc(UC_ARCH_ARM, UC_MODE_ARM)
for address, size in ((0x08000000, 0x2000000), (0x02000000, 0x40000),
                      (0x03000000, 0x8000), (0x04000000, 0x10000),
                      (0x06000000, 0x20000), (0x01000000, 0x1000)):
    cpu.mem_map(address, size)
cpu.mem_write(0x08000000, (a.out / "prototype.gba").read_bytes())
cpu.mem_write(0x02000000, bytes([0xa5]) * 0x40000)
cpu.mem_write(0x03000000, bytes([0xa5]) * 0x8000)
cpu.mem_write(0x06000000, bytes([0xa5]) * 0x20000)
abi = struct.pack("<4I", 4096, 0x08800000, 0xfab0babe, 2) + bytes(range(80))
cpu.mem_write(0x0203fbf8, abi)
waitcnt = []
cpu.hook_add(UC_HOOK_MEM_WRITE, lambda u, access, address, size, value, data:
             waitcnt.append((size, value)), begin=0x04000204, end=0x04000205)
cpu.emu_start(0x08000000, symbols["main"] & ~1, count=1000000)
assert cpu.reg_read(UC_ARM_REG_PC) == (symbols["main"] & ~1)
assert bytes(cpu.mem_read(0x0203fbf8, 96)) == abi
assert bytes(cpu.mem_read(0x02030000, 16)) == bytes([0xa5]) * 16
assert bytes(cpu.mem_read(0x06000000, 0x18000)) == bytes(0x18000)
assert bytes(cpu.mem_read(0x06018000, 16)) == bytes([0xa5]) * 16
assert waitcnt == [(2, 0x4000)], waitcnt
assert int.from_bytes(cpu.mem_read(symbols["screen"], 4), "little") == 0x06000000
for lo, hi in (("__bss_start__", "__bss_end__"), ("__sbss_start__", "__sbss_end__")):
    assert bytes(cpu.mem_read(symbols[lo], symbols[hi] - symbols[lo])) == bytes(symbols[hi] - symbols[lo])

cases = [(2048, 0x08800000, 0xfab0babe, 2, True),
         (4096, 0x08801000, 0xfab0babe, 2, True),
         (4096, 0x08dff000, 0xfab0babe, 2, True),
         (8192, 0x08800000, 0xfab0babe, 2, True),
         (16384, 0x08800000, 0xfab0babe, 2, True)]
cases += [(n, 0x08800000, 0xfab0babe, 2, False) for n in (0, 2047, 4095, 8193, 16385)]
cases += [(4096, n, 0xfab0babe, 2, False)
          for n in (0, 0x02000000, 0x07fffffc, 0x08e00000, 0x08dff004, 0x08800002, 0xffffffff)]
cases += [(4096, 0x08800000, 0, 2, False)]
cases += [(4096, 0x08800000, 0xfab0babe, n, False) for n in (0, 1, 3)]
for size, address, magic, argc, valid in cases:
    cpu.mem_write(0x0203fbf8, struct.pack("<4I", size, address, magic, argc))
    cpu.mem_write(symbols["cart"], bytes([0xcc]) * 16384)
    cpu.mem_write(symbols["theRom"], bytes([0xcc]) * 4096)
    pattern = bytes((i * 17 + 9 + (i // 4096) * 37) & 255 for i in range(size)) if valid else b""
    if valid:
        cpu.mem_write(address, pattern)
    cpu.reg_write(UC_ARM_REG_R0, 0)
    cpu.reg_write(UC_ARM_REG_SP, 0x03007e00)
    cpu.reg_write(UC_ARM_REG_LR, 0x01000001)
    cpu.emu_start(symbols["loadCart"] | 1, 0x01000000, count=100000)
    assert cpu.reg_read(UC_ARM_REG_PC) == 0x01000000
    assert cpu.reg_read(UC_ARM_REG_R0) == (0 if valid else 0xffffffff)
    if valid:
        assert bytes(cpu.mem_read(symbols["cart"], 16384)) == pattern + bytes([0xcc]) * (16384 - size)
        assert bytes(cpu.mem_read(symbols["theRom"], 4096)) == (pattern * (4096 // size) if size <= 4096 else pattern[-4096:])
    else:
        assert bytes(cpu.mem_read(symbols["cart"], 16384)) == bytes([0xcc]) * 16384
        assert bytes(cpu.mem_read(symbols["theRom"], 4096)) == bytes([0xcc]) * 4096
# Invoke the core's real F8/F6 read/write hotspots, with distinct data per bank.
for size, hotspots in ((8192, (0x1ff8, 0x1ff9)),
                       (16384, (0x1ff6, 0x1ff7, 0x1ff8, 0x1ff9))):
    pattern = b"".join(bytes([bank + 17]) * 4096 for bank in range(size // 4096))
    cpu.mem_write(0x08800000, pattern)
    cpu.mem_write(0x0203fbf8, struct.pack("<4I", size, 0x08800000, 0xfab0babe, 2))
    cpu.reg_write(UC_ARM_REG_LR, 0x01000001)
    cpu.emu_start(symbols["loadCart"] | 1, 0x01000000, count=100000)
    for operation in ("decRead", "decWrite"):
        for bank, hotspot in enumerate(hotspots):
            cpu.reg_write(UC_ARM_REG_R0, hotspot)
            cpu.reg_write(UC_ARM_REG_LR, 0x01000001)
            cpu.emu_start(symbols[operation] | 1, 0x01000000, count=100000)
            assert bytes(cpu.mem_read(symbols["theRom"], 4096)) == bytes([bank + 17]) * 4096

# An external emulator must refuse a missing, malformed or oversized return image.
for magic, tail, size, marker, valid in (
    (0x45505553, 0x00574652, 512, 0x54525344, True),
    (0x45505553, 0x00574652, 0x200000, 0x54525344, True),
    (0, 0x00574652, 512, 0x54525344, False),
    (0x45505553, 0, 512, 0x54525344, False),
    (0x45505553, 0x00574652, 512, 0, False),
    (0x45505553, 0x00574652, 256, 0x54525344, False),
    (0x45505553, 0x00574652, 0x200200, 0x54525344, False)):
    cpu.mem_write(0x098000a0, struct.pack("<2I", magic, tail))
    cpu.mem_write(0x098000cc, struct.pack("<I", size))
    cpu.mem_write(0x09a00004, struct.pack("<I", marker))
    cpu.reg_write(UC_ARM_REG_LR, 0x01000001)
    cpu.emu_start(symbols["atari_return_ready"] | 1, 0x01000000, count=10000)
    assert bool(cpu.reg_read(UC_ARM_REG_R0)) == valid
record = {"startup": "actual ARM startup preserves all 96 ABI bytes and unowned EWRAM; owned VRAM/BSS cleared, data copied, WAITCNT=0x4000",
          "cartridge_cases": len(cases), "bank_hotspots": 12, "return_header_cases": 7, "result": "passed", "limits": "CPU/bus model, not physical SuperCard or game compatibility qualification"}
(a.out / "input-validation.json").write_text(json.dumps(record, indent=2) + "\n")
print(f"Atari actual ARM startup and {len(cases)} runtime input/mirror/bounds cases passed.")
