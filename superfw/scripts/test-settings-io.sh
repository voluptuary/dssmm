#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
gcc -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie \
 -I "$root" -I "$root/src" "$root/tests/settings-io-test.c" "$root/src/settings_io.c" "$root/src/menu_workspace.c" -o "$tmp/test"
"$tmp/test"

for board in sd lite chis; do
 flags=();case "$board" in lite) flags=(-DSUPERCARD_LITE_IO);; chis) flags=(-DSUPERCHIS_IO);; esac
 gcc -O1 -g -fshort-enums -ffunction-sections -fdata-sections -fsanitize=address,undefined -fno-pie -no-pie "${flags[@]}" \
  -I "$root" -I "$root/src" "$root/tests/settings-roundtrip-test.c" "$root/src/settings_io.c" "$root/src/menu_workspace.c" \
  "$root/src/launcher_profile.c" "$root/src/util.c" "$root/src/fileutil.c" "$root/src/nanoprintf.c" \
  -Wl,--gc-sections -o "$tmp/roundtrip"
 "$tmp/roundtrip"
done

gcc -O1 -g -fshort-enums -ffunction-sections -fdata-sections -fsanitize=address,undefined -fno-pie -no-pie \
 -I "$root" -I "$root/src" "$root/tests/settings-fatfs-test.c" \
 "$root/src/settings.c" "$root/src/settings_io.c" "$root/src/menu_workspace.c" "$root/src/launcher_profile.c" \
 "$root/src/util.c" "$root/src/fileutil.c" "$root/src/nanoprintf.c" \
 "$root/fatfs/ff.c" "$root/fatfs/ffunicode.c" "$root/fatfs/ffsystem.c" \
 -Wl,--gc-sections -o "$tmp/fatfs"
"$tmp/fatfs"
