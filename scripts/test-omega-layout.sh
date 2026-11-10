#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
nm_bin="${NM:-${DEVKITARM:-/opt/devkitpro/devkitARM}/bin/arm-none-eabi-nm}"
python3 - "$nm_bin" "${1:-$repo_root/omega-port.elf}" <<'PY'
import subprocess,sys
symbols={}
for line in subprocess.check_output([sys.argv[1],sys.argv[2]],text=True).splitlines():
 parts=line.split()
 if len(parts)==3:
  try:symbols[parts[2]]=int(parts[0],16)
  except ValueError:pass
s=symbols
assert 0x02000000 <= s['__sbss_end__'] <= s['__eheap_start'] < s['__eheap_end'] <= 0x02040000
assert 0x03000000 <= s['__data_end__'] < s['__sp_usr'] < s['__sp_irq'] < s['__irq_flags'] < 0x03008000
print('Omega layout: EWRAM heap',s['__eheap_end']-s['__eheap_start'],'bytes; IWRAM data-to-user-SP gap',s['__sp_usr']-s['__data_end__'],'bytes (static layout, not runtime stack proof).')
PY
