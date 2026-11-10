#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$repo_root/source/GBApatch.c" "$tmp/omega-patch.impl.h" <<'PY'
import re,sys
from pathlib import Path
s=Path(sys.argv[1]).read_text(errors='surrogateescape');out=[]
for name in ['Write','SetTrimSize']:
 m=re.search(r'^void '+name+r'\([^;]*?\n\{',s,re.M);assert m
 out.append(s[m.start():s.index('\n}',m.start())+2])
Path(sys.argv[2]).write_text('\n'.join(out),errors='surrogateescape')
PY
"${CC:-cc}" -std=c99 -O1 -Wall -Wextra -Werror -fsanitize=undefined -I"$tmp" "$repo_root/tests/omega/patch-boundaries.c" -o "$tmp/test"
"$tmp/test"
