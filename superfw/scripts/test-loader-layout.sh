#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
gcc -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie \
 -I "$root" -I "$root/src" "$root/tests/loader-layout-test.c" "$root/src/loader_layout.c" -o "$tmp/layout"
"$tmp/layout"
python3 - "$root/src/loader.c" "$tmp/gba-load-under-test.h" <<'PY'
from pathlib import Path
import sys
source=Path(sys.argv[1]).read_text();parts=[]
for prefix in ['static void rom_write16_exact(', 'unsigned load_gba_rom(']:
 start=source.index(prefix);end=source.index('\n}',start)+2;parts.append(source[start:end])
Path(sys.argv[2]).write_text('\n'.join(parts))
PY
gcc -O1 -g -Wall -Wextra -Werror -Wno-unused-function -fsanitize=address,undefined -fno-pie -no-pie \
 -I "$root" -I "$root/src" -I "$tmp" "$root/tests/gba-loader-test.c" "$root/src/loader_layout.c" -o "$tmp/load"
"$tmp/load"
