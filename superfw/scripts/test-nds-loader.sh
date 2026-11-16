#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
gcc -O1 -g -Wall -Wextra -Werror -Wno-unused-function -Wno-sign-compare -fsanitize=address,undefined -fno-pie -no-pie \
 -ffunction-sections -fdata-sections -I "$root" -I "$root/src" "$root/tests/nds-loader-test.c" -Wl,--gc-sections -o "$tmp/test"
"$tmp/test"
