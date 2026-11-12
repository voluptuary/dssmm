#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
awk '/^static void NOR_DrawWriteProgress\(/{p=1} p{print} p&&/^}$/{exit}' "$repo_root/source/NORflash_OP.c" > "$tmp/omega-progress.impl.h"
"${CC:-cc}" -std=c99 -Wall -Wextra -Werror -fsanitize=undefined -I"$tmp" "$repo_root/tests/omega/nor-progress.c" -o "$tmp/test"
"$tmp/test"
