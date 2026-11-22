#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
gcc -O1 -g -Wall -Wextra -Werror -Wno-sign-compare -fsanitize=address,undefined -fno-pie -no-pie \
 -ffunction-sections -fdata-sections -I "$root" -I "$root/src" \
 "$root/tests/patchengine-bounds-test.c" -Wl,--gc-sections -o "$tmp/test"
"$tmp/test"
python3 - "$root/src/menu.c" "$tmp/patch-load-under-test.h" <<'PY'
from pathlib import Path
import sys
source=Path(sys.argv[1]).read_text();start=source.index('bool generate_patches_progress(')
end=source.index('\n}',start)+2
declarations=source[source.index('enum progress_action {'):source.index('static void render_progress_panel(')]
Path(sys.argv[2]).write_text(declarations+source[start:end])
PY
gcc -O1 -g -Wall -Wextra -Werror -Wno-unused-function -Wno-misleading-indentation -fsanitize=address,undefined -fno-pie -no-pie \
 -I "$root" -I "$root/src" -I "$tmp" "$root/tests/patch-load-test.c" -o "$tmp/loader"
"$tmp/loader"
