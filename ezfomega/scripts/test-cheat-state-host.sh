#!/bin/sh
set -eu
repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
test_tmp=$(mktemp -d)
trap 'rm -rf "$test_tmp"' EXIT HUP INT TERM
python3 - "$repo_root/source/showcht.c" "$test_tmp/impl.c" <<'PY'
import sys
from pathlib import Path
s=Path(sys.argv[1]).read_text()
parts=[s[s.index('#define CHEAT_SELECTION_CACHE_MAX'):s.index('extern u16 gl_select_lang;')],
       s[s.index('/* CHS2'):s.index('static u32 CheatTextLooksGB2312')],
       s[s.index('u32 GetVisibleCheatMenuCount'):s.index('//------------------------------------------------------------------\nunsigned long str2hex')]]
Path(sys.argv[2]).write_text('\n'.join(parts))
PY
cc -std=gnu11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie \
  -DCHEAT_IMPL="\"$test_tmp/impl.c\"" "$repo_root/tests/cheat-state-host.c" -o "$test_tmp/test"
"$test_tmp/test"
