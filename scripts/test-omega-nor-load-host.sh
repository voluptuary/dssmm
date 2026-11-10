#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
awk '/^u32 Loadfile2NOR\(/{p=1} p{print} p&&/^}$/{exit}' "$repo_root/source/NORflash_OP.c" > "$tmp/omega-nor.impl.h"
"${CC:-cc}" -std=gnu99 -Wall -Wextra -Wno-format -Wno-unused-parameter -fsanitize=undefined \
 -I"$tmp" -I"$repo_root/source/ff15" "$repo_root/tests/omega/nor-load.c" -o "$tmp/test"
"$tmp/test"
