#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
awk '/^u32 GetFileListFromNor\(/{p=1} p{print} p&&/^}$/{exit}' "$repo_root/source/NORflash_OP.c" > "$tmp/omega-scan.impl.h"
# Represent the cartridge address as a native host pointer; algorithm unchanged.
sed -i 's/u32 StartAddress/uintptr_t StartAddress/' "$tmp/omega-scan.impl.h"
"${CC:-cc}" -std=c99 -Wall -Wextra -Werror -fsanitize=undefined -I"$tmp" "$repo_root/tests/omega/nor-scan.c" -o "$tmp/test"
"$tmp/test"
