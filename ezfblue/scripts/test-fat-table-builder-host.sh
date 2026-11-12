#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
test_tmp=$(mktemp -d)
trap 'rm -rf "$test_tmp"' EXIT HUP INT TERM

awk '/^u32 Check_game_RTS_FAT\(/{p=1} p{print} p&&/^}$/{exit}' \
	"$repo_root/source/ezkernelnew.c" > "$test_tmp/check-game-rts-fat.impl.c"
[ -s "$test_tmp/check-game-rts-fat.impl.c" ] || {
	echo "FAIL: could not extract Check_game_RTS_FAT()" >&2
	exit 1
}

gcc -std=c99 -O2 -Wall -Wextra \
	-I"$repo_root/source" \
	-DCHECK_GAME_RTS_FAT_IMPL="\"$test_tmp/check-game-rts-fat.impl.c\"" \
	"$repo_root/tests/fat-table-builder-host.c" \
	-o "$test_tmp/fat-table-builder-host"

"$test_tmp/fat-table-builder-host"
