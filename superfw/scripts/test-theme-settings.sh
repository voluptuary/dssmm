#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
test_dir="$(mktemp -d)"
trap 'rm -rf "$test_dir"' EXIT
for variant in native chis; do
  flags=()
  if [ "$variant" = chis ]; then flags=(-DSUPERCHIS_IO); fi
  gcc -O0 -fshort-enums -ffunction-sections -fdata-sections "${flags[@]}" \
    -I "$project_root" -I "$project_root/src" \
    "$project_root/tests/theme-settings-test.c" "$project_root/src/util.c" "$project_root/src/fileutil.c" "$project_root/src/nanoprintf.c" \
    -Wl,--gc-sections -o "$test_dir/theme-test"
  "$test_dir/theme-test"
done
printf 'Theme defaults, legacy IDs and configuration bounds passed.\n'
