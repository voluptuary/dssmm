#!/usr/bin/env python3
"""Optional mGBA regression: load a generated old-Jagoomba battery save in MiGoomba.

Needs the pinned upstream checkout with tools/gbtest/build/gbaharness compiled
and public test ROMs fetched. No commercial ROMs or user saves are used.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--upstream', required=True, type=Path)
args = parser.parse_args()
up = args.upstream.resolve()
meta = json.loads((ROOT / 'docs/plugins/migoomba.json').read_text())
assert subprocess.check_output(['git', '-C', str(up), 'rev-parse', 'HEAD'], text=True).strip() == meta['upstream_commit']
binary = up / meta['upstream_binary']
assert hashlib.sha256(binary.read_bytes()).hexdigest() == meta['sha256']
sys.path.insert(0, str(up / 'tools/gbtest'))
import savetest
from harness import Harness
old_meta = meta['previous_payload']
old_header = subprocess.check_output(['git', '-C', str(ROOT), 'show', old_meta['commit'] + ':' + old_meta['header']]).decode()
old = bytes(int(value, 16) for value in re.findall(r'0x([0-9A-Fa-f]{2})\b', old_header))
assert len(old) == old_meta['bytes'] and hashlib.sha256(old).hexdigest() == old_meta['sha256']
with tempfile.TemporaryDirectory(prefix='migoomba-save-compat-') as folder:
    work = Path(folder)
    rom = work / 'test.gb'
    logo = up / 'tools/gbtest/roms/mooneye-test-suite/emulator-only/mbc2/ram.gb'
    savetest.build_rom(str(rom), str(logo), 'mbc5_32k')
    old_gba = work / 'old.gba'
    old_gba.write_bytes(old + rom.read_bytes())
    save = work / 'game.esv'
    h = Harness(str(old_gba), save=str(save), sram_kb=64)
    try:
        h.frames(600)
        h.cmd('keys 300')
        h.frames(90)
        h.cmd('keys 0')
        h.frames(90)
    finally:
        h.close()
    plain = work / 'old.sav'
    subprocess.run(['python3', str(up / 'tools/goombasave.py'), 'export', str(save), str(rom), str(plain)], check=True, stdout=subprocess.DEVNULL)
    assert plain.read_bytes()[0x6000] & 15 == 1
    new_gba = work / 'new.gba'
    new_gba.write_bytes(binary.read_bytes() + rom.read_bytes())
    elf = binary.with_suffix('.elf')
    session = savetest.Session(str(new_gba), str(elf), str(save), 64)
    try:
        assert session.report() == 1
        assert session.press_a() == 2
    finally:
        session.close()
    session = savetest.Session(str(new_gba), str(elf), str(save), 64)
    try:
        assert session.report() == 2
    finally:
        session.close()
    assert save.stat().st_size == 0x10000
print('Legacy Jagoomba -> MiGoomba: generated MBC5 battery save loads and survives a simulated power cycle; old saved states not covered.')
