#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 "$root/tools/generate-dssmm-font.py" --check
gcc -std=c11 -O2 -Wall -Wextra -Werror -Wno-old-style-declaration \
 -fsanitize=address,undefined -fno-pie -no-pie -I "$root/src" -I "$root/src/fonts" \
 "$root/tests/dssmm-font-test.c" "$root/src/fonts/font_render.c" "$root/src/utf_util.c" -o "$tmp/test"
"$tmp/test" "$root/assets/fonts/dssmm-ascii-rows.bin"
