#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$repo_root/source/ezkernelnew.c" "$tmp/omega-artwork.impl.h" <<'PY'
import sys
from pathlib import Path
s=Path(sys.argv[1]).read_text(); out=[]
for name in ['Launcher_LoadFilenameThumbnail','Load_ThumbnailEx']:
 start=s.rfind('\n',0,s.index(name+'(',s.index('static u32 Launcher_LoadFilenameThumbnail')))+1
 end=s.index('\n}',start)+2
 out.append(s[start:end])
Path(sys.argv[2]).write_text('\n'.join(out))
PY
"${CC:-cc}" -std=c99 -Wall -Wextra -Werror -fsanitize=undefined -I"$tmp" "$repo_root/tests/omega/artwork-fallback.c" -o "$tmp/test"
"$tmp/test"
