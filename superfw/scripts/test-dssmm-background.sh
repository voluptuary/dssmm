#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 "$root/tools/generate-dssmm-background.py" --check
gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie \
 -I "$root/src" "$root/tests/dssmm-background-test.c" \
 "$root/src/dssmm_background.c" -o "$tmp/test"
assets=()
for theme in light dark; do
 for kind in start settings horizontal vertical grid; do
  assets+=("$root/assets/backgrounds/$theme-$kind.rgb555")
 done
done
"$tmp/test" "${assets[@]}"
