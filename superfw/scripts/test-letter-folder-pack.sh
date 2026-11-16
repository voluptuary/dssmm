#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 "$root/tools/generate-letter-folders.py" "$tmp/pack"
gcc -O2 -Wall -Wextra -Werror -Wno-sign-compare -fsanitize=address,undefined -fno-pie -no-pie \
 -I "$root/src" "$root/tests/letter-folder-pack-test.c" "$root/src/artwork.c" -o "$tmp/test"
"$tmp/test" "$tmp/pack"
