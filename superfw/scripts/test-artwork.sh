#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
test_dir="$(mktemp -d)"
trap 'rm -rf "$test_dir"' EXIT
gcc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -g \
  -I "$project_root/src" "$project_root/tests/artwork-test.c" \
  "$project_root/src/artwork.c" -o "$test_dir/artwork-test"
"$test_dir/artwork-test"
printf 'Artwork dimensions, channels, transparency, bounds and read faults passed.\n'
