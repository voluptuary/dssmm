#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
test_dir="$(mktemp -d)"
trap 'rm -rf "$test_dir"' EXIT
for variant in sd lite chis; do
  flags=()
  case "$variant" in
    lite) flags=(-DSUPERCARD_LITE_IO) ;;
    chis) flags=(-DSUPERCHIS_IO) ;;
  esac
  gcc -O0 -fshort-enums -ffunction-sections -fdata-sections "${flags[@]}" \
    -I "$project_root" -I "$project_root/src" \
    "$project_root/tests/theme-settings-test.c" "$project_root/src/util.c" "$project_root/src/fileutil.c" "$project_root/src/nanoprintf.c" \
    -Wl,--gc-sections -o "$test_dir/theme-test"
  "$test_dir/theme-test"
done
printf 'SD, Lite and Chis theme defaults, migrations, legacy IDs and configuration bounds passed.\n'
