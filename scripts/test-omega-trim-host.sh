#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
awk '/^static u32 Launcher_LoadTrimSize\(/{p=1} p{print} p&&/^}$/{exit}' "$repo_root/source/ezkernelnew.c" > "$tmp/omega-trim.impl.h"
"${CC:-cc}" -std=c99 -Wall -Wextra -Werror -fsanitize=undefined -I"$tmp" -I"$repo_root/source/ff15" "$repo_root/tests/omega/trim-io.c" -o "$tmp/test"
"$tmp/test"
