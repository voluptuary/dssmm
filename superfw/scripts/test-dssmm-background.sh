#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 "$root/tools/generate-dssmm-background.py" --check
gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie \
 -I "$root/src" "$root/tests/dssmm-background-test.c" \
 "$root/src/dssmm_background.c" -o "$tmp/test"
"$tmp/test" "$root/assets/backgrounds/light-start.rgb555" "$root/assets/backgrounds/dark-start.rgb555"
