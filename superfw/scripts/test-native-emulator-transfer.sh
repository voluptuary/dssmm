#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "${DSSMM_TEST_LOADER_SOURCE:-$root/src/loader.c}" "$tmp/native-emulator.h" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text();a=s.index('unsigned load_extemu_rom(');b=s.index('\n}',a)+2
helper=s.index('static bool native_emulator_copy(') if 'static bool native_emulator_copy(' in s else a
Path(sys.argv[2]).write_text(s[helper:b].replace('NOINLINE',''))
PY
gcc -O1 -g -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-variable -fsanitize=address,undefined -fno-pie -no-pie -I "$root" -I "$root/src" -I "$tmp" "$root/tests/native-emulator-transfer-test.c" -o "$tmp/test"
"$tmp/test"
