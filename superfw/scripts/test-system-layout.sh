#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie \
 -I "$root" -I "$root/src" "$root/tests/system-layout-test.c" \
 "$root/src/system_layout.c" "$root/src/nanoprintf.c" -o "$tmp/test"
"$tmp/test"
