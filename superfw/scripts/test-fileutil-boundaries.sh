#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
gcc -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie \
 -ffunction-sections -fdata-sections -I "$root" -I "$root/src" \
 "$root/tests/fileutil-boundary-test.c" "$root/src/fileutil.c" -Wl,--gc-sections -o "$tmp/test"
"$tmp/test"
printf 'Maximum-length parent paths fit; oversized and empty paths are rejected safely.\n'
