#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
python3 "$root/tests/test-launch-io-host.py" --case copy
python3 "$root/tests/test-launch-io-host.py" --case metadata
python3 - "$root" <<'CHECK'
import re, sys
from pathlib import Path
source=(Path(sys.argv[1])/'source/ezkernelnew.c').read_text()
for name in ['LoadEMU2PSRAM','Loadfile2PSRAM']:
    assert re.search(r'res\s*=\s*'+name+r'\(pfilename(?:,is_EMU)?\);\s*if\(res != 0\)\{\s*Show_error_num\(1\);\s*return 1;',source)
assert 'make_pogoshell_arguments(plugin + 9' not in source
print('DE load errors request index rebuild and plugin arguments use basename.')
CHECK
