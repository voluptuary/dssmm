#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 "$root/tools/generate-dssmm-panel.py" --check
python3 - "$root/src/menu.c" "$tmp/panel-under-test.h" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text();a=s.index('static void draw_lined_panel(');b=s.index('\n}',a)+2
Path(sys.argv[2]).write_text(s[a:b])
PY
gcc -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie \
 -I "$root/src" -I "$tmp" "$root/tests/dssmm-panel-test.c" "$root/src/dssmm_background.c" -o "$tmp/test"
"$tmp/test" "$root/assets/backgrounds/light-settings.rgb555" "$root/assets/backgrounds/dark-settings.rgb555"
