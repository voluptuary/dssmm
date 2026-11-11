#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$repo_root/source/ezkernelnew.c" "$tmp/omega-artwork.impl.h" <<'PY'
import sys
from pathlib import Path
s=Path(sys.argv[1]).read_text(); out=[]
start=s.index('typedef struct {\n\tFILINFO info;')
end=s.index('\nu32 Load_ThumbnailEx',start)
out.append(s[start:end])
start=s.rfind('\n',0,s.index('Load_ThumbnailEx(',end))+1
out.append(s[start:s.index('\n}',start)+2])
start=s.index('static u32 Launcher_LoadThumbDataForIndex(')
out.append(s[start:s.index('\n}',start)+2])
Path(sys.argv[2]).write_text('\n'.join(out))
PY
"${CC:-cc}" -std=c99 -Wall -Wextra -Werror -fsanitize=undefined -I"$tmp" -I"$repo_root/source" "$repo_root/tests/omega/artwork-fallback.c" -o "$tmp/test"
"$tmp/test"
