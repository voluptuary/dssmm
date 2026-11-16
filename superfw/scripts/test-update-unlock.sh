#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie \
 -I "$root/src" "$root/tests/update-unlock-test.c" "$root/src/update_unlock.c" -o "$tmp/test"
"$tmp/test"
printf 'Update sequence order, five-second bounds, wrong keys/chords, restart and frame wrap passed.\n'
