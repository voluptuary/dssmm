#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
gcc -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie \
 -I "$root" -I "$root/src" "$root/tests/settings-io-test.c" "$root/src/settings_io.c" -o "$tmp/test"
"$tmp/test"
