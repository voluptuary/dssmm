#!/usr/bin/env python3
"""Check all BLUE ELF load sections against the resource and metadata reservations."""
import argparse
import importlib.util
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('blue_layout_probe', ROOT / 'scripts/probe-blue-layout.py')
probe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(probe)


def validate(elf, code=None):
    objdump = Path(os.environ['DEVKITARM']) / 'bin/arm-none-eabi-objdump'
    table = probe.sections(Path(elf), objdump)
    elf_bytes = Path(elf).read_bytes()
    loaded = []
    for section in table:
        if not section['size'] or not all(flag in section['flags'] for flag in ('CONTENTS', 'ALLOC', 'LOAD')):
            continue
        start = section['lma'] - probe.ROM
        end = start + section['size']
        probe.require_clear(start, end)
        if code is not None:
            if end > len(code):
                raise ValueError('Raw BLUE code omits an ELF load section')
            # gbafix edits only the cartridge header, not executable/asset contents.
            skip = max(0, 0xC0 - start) if start < 0xC0 else 0
            expected = elf_bytes[section['offset'] + skip:section['offset'] + section['size']]
            if code[start + skip:end] != expected:
                raise ValueError('Raw BLUE bytes do not match ELF section ' + section['name'])
        loaded.append((section['name'], start, end))
    if not loaded or not any(name == '.rodata' and start == probe.ASSET_START for name, start, end in loaded):
        raise ValueError('BLUE split resource layout is absent')
    if code is not None and len(code) != max(end for name, start, end in loaded):
        raise ValueError('Unexpected trailing/truncated BLUE image contents')
    return loaded


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('elf', type=Path)
    args = parser.parse_args()
    result = validate(args.elf, args.elf.with_suffix('.gba').read_bytes())
    print(f'BLUE split layout: {max(end for name, start, end in result)} bytes; fixed resources and metadata protected.')
