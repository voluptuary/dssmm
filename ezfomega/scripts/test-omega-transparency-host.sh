#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$repo_root/source/ezkernelnew.c" "$tmp/omega-alpha.impl.h" <<'PY'
import re,sys
from pathlib import Path
s=Path(sys.argv[1]).read_text();out=[]
for name in ['Launcher_HasTransparentThumbnailMarker','Launcher_TagTransparentThumbnailPixels']:
 m=re.search(r'^static [^;\n]*\b'+name+r'\([^;]*?\n\{',s,re.M);assert m
 out.append(s[m.start():s.index('\n}',m.start())+2])
Path(sys.argv[2]).write_text('\n'.join(out))
PY
"${CC:-cc}" -std=c99 -Wall -Wextra -Werror -fsanitize=undefined -I"$tmp" "$repo_root/tests/omega/transparency-loader.c" -o "$tmp/test"
"$tmp/test"
bash "$repo_root/scripts/test-launcher-artwork-draw-host.sh" 5f700c9
