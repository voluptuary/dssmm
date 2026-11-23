#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$root/src/menu.c" "$tmp/carousel-source.h" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text();a=s.index('static bool carousel_source_nor(');b=s.index('/* One bounded SD read',a)
Path(sys.argv[2]).write_text(s[a:b].replace('#pragma GCC pop_options',''))
PY
for nor in 0 1; do
 gcc -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -DTEST_NOR="$nor" -I"$root/src" -I"$tmp" "$root/tests/carousel-source-test.c" -o "$tmp/test"
 "$tmp/test"
done
printf 'Shared carousel source: SD/NOR paths, icons, counts and artwork priority passed.\n'
