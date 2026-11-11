#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
test_tmp=$(mktemp -d)
trap 'rm -rf "$test_tmp"' EXIT HUP INT TERM

awk '
	/^static inline .*Launcher_(ArtBorderEnabled|RoundedCornersForCustom)\(/ && $0 !~ /;[ \t]*$/ { p=1 }
	p { print }
	p && /^}/ { print ""; p=0; found++ }
	END { if(found != 2) exit 1 }
' "$repo_root/source/ezkernelnew.c" > "$test_tmp/policy.inc"
[ -s "$test_tmp/policy.inc" ] || { echo "FAIL: could not extract artwork style policy" >&2; exit 1; }

gcc -std=gnu99 -O2 -Wall -Wextra -Werror \
	-DPOLICY_FILE="\"$test_tmp/policy.inc\"" \
	"$repo_root/tests/launcher-art-style-policy-host.c" \
	-o "$test_tmp/launcher-art-style-policy-host"

"$test_tmp/launcher-art-style-policy-host"
