#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$root/src/menu.c" "$tmp/nor-actions.h" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text(); parts=[]
for name in ['static unsigned nor_write_blocks(', 'static void confirm_nor_write(', 'static void request_nor_write(void) {']:
 start=s.index(name); end=s.index('\n}',start)+2
 body=s[start:end]
 if name.startswith('static void confirm'):
  # Exercise production confirmation/preflight/capacity flow without issuing flash writes.
  body=body[:body.index('  const t_load_gba_info *info')]+'  writes++;\n}'
 parts.append(body)
Path(sys.argv[2]).write_text('\n'.join(parts))
PY
gcc -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie -I "$tmp" "$root/tests/nor-launch-test.c" -o "$tmp/test"
"$tmp/test"
python3 - "$root/src/loader.c" "$tmp/nor-loader.h" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text();start=s.index('unsigned flash_gba_nor(');end=s.index('\n}',start)+2
Path(sys.argv[2]).write_text(s[start:end])
PY
gcc -Wall -Wextra -Werror -Wno-unused-parameter -fsanitize=address,undefined -fno-pie -no-pie -I "$tmp" "$root/tests/nor-loader-test.c" -o "$tmp/loader"
"$tmp/loader"
python3 - "$root/src/menu.c" "$tmp/nor-payload.h" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text();parts=[]
for name in ['bool ingame_menu_avail_flash(', 'bool dirsav_avail_flash(']:
 start=s.index(name);end=s.index('\n}',start)+2;parts.append(s[start:end])
Path(sys.argv[2]).write_text('\n'.join(parts))
PY
gcc -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie -I "$tmp" "$root/tests/nor-payload-test.c" -o "$tmp/payload"
"$tmp/payload"
