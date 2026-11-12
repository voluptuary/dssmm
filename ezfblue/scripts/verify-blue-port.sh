#!/usr/bin/env bash
set -euo pipefail
repo_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_dir"
python3 tests/test-migoomba-payload.py
make
make check
bash scripts/test-blue-ui.sh
bash scripts/test-marked-artwork-host.sh
bash scripts/test-oem-path-host.sh
bash scripts/test-save-load-host.sh
python3 - <<'PY'
from pathlib import Path
import importlib.util
import os
import subprocess

root = Path.cwd()
spec = importlib.util.spec_from_file_location('blue_packer', root / 'scripts/pack-blue-kernel.py')
packer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(packer)
code = root / 'targets/blue/ezbluekernel-development.gba'
linked = packer.pack(code.read_bytes(), elf=code.with_suffix('.elf'))
print(f'BLUE firmware-free stock-size image verified in memory: {len(linked)} bytes; no update distributed.')

nm = Path(os.environ['DEVKITARM']) / 'bin/arm-none-eabi-nm'
symbols = {}
for line in subprocess.check_output([str(nm), '-n', str(code.with_suffix('.elf'))], text=True).splitlines():
    fields = line.split()
    if len(fields) == 3:
        symbols[fields[2]] = int(fields[0], 16)
heap_start, heap_end = symbols['__eheap_start'], symbols['__eheap_end']
iwram_end, stack = symbols['__iwram_overlay_end'], symbols['__sp_usr']
assert 0x02000000 <= heap_start < heap_end <= 0x02040000
assert 0x03000000 <= iwram_end < symbols['__sp_irq'] <= 0x03008000
assert symbols['__sp_irq'] - iwram_end >= 1024
assert stack == 0x0203F000 and heap_end <= stack - 0x3000
irq_gap = symbols['__sp_irq'] - iwram_end
print(f'BLUE static layout: EWRAM heap {heap_end - heap_start}; IWRAM-to-IRQ-SP gap {irq_gap}.')
print('Not a runtime allocation/stack bound or hardware compatibility result.')
PY
