#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
test_dir="$(mktemp -d)"
trap 'rm -rf "$test_dir"' EXIT
gcc -O2 -Wall -Wextra -Werror -I "$project_root/src" \
  "$project_root/tests/browser-motion-test.c" "$project_root/src/browser_carousel.c" \
  -o "$test_dir/browser-motion-test"
"$test_dir/browser-motion-test"
printf 'Carousel motion retargeting, frame steps and bounds passed.\n'
python3 "$project_root/tests/test-browser-fixtures.py"
