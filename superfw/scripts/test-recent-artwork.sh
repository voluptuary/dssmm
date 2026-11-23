#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$root/src/menu.c" "$tmp/recent-art.h" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text();a=s.index('static bool recent_artwork_load(');b=s.index('\n}',a)+2
Path(sys.argv[2]).write_text(s[a:b])
PY
for nor in 0 1; do
 gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -DTEST_NOR="$nor" -I"$root/src" -I"$tmp" "$root/tests/recent-artwork-test.c" -o "$tmp/test"
 "$tmp/test"
done
printf 'Production recent artwork: SD/NOR priority, stored codes, missing metadata and Box guards passed.\n'
