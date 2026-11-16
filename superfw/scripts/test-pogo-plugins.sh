#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
test_dir="$(mktemp -d)"
trap 'rm -rf "$test_dir"' EXIT
python3 - "$project_root" "$test_dir" <<'PY'
import pathlib,sys
root,out=map(pathlib.Path,sys.argv[1:])
s=(root/'src/loader.c').read_text();start=s.index('static void rom_write16_exact(')
end=s.index('\n}',start)+2
helper=s[start:end]
start=s.index('static unsigned load_pogo_plugin(');end=s.index('// Generic-emulator',start)
(out/'pogo-loader-under-test.h').write_text(helper+'\n'+s[start:end])
PY
for board in sd lite chis; do
  flags=()
  case "$board" in lite) flags=(-DSUPERCARD_LITE_IO);; chis) flags=(-DSUPERCHIS_IO);; esac
  gcc -g -O1 -fsanitize=address,undefined -fno-pie -no-pie "${flags[@]}" \
    -I "$project_root" -I "$test_dir" -I "$project_root/src" "$project_root/tests/pogo-plugin-test.c" \
    "$project_root/src/pogo_plugin.c" "$project_root/src/emu.c" "$project_root/src/util.c" \
    -ffunction-sections -fdata-sections -Wl,--gc-sections -o "$test_dir/test"
  "$test_dir/test"
done
