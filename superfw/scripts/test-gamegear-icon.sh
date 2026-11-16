#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 "$root/tools/generate-gamegear-icon.py" --check
python3 - "$root/src/menu.c" "$tmp/icon-lookup-under-test.h" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text();a=s.index('static unsigned guessicon(');b=s.index('\n}',a)+2
Path(sys.argv[2]).write_text(s[a:b])
PY
for board in sd lite chis; do
 flags=();case "$board" in lite) flags=(-DSUPERCARD_LITE_IO);; chis) flags=(-DSUPPORT_NORGAMES);; esac
 gcc -O1 -g -fsanitize=address,undefined -fno-pie -no-pie -ffunction-sections -fdata-sections "${flags[@]}" \
  -I "$tmp" -I "$root/src" "$root/tests/gamegear-icon-test.c" "$root/src/util.c" \
  -Wl,--gc-sections -o "$tmp/test"
 "$tmp/test" "$root/assets/icons/gamegear.rgb555"
done
