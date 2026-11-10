#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
test_bin="$(mktemp "${TMPDIR:-/tmp}/psram-load-policy-host.XXXXXX")"
trap 'rm -f "$test_bin"' EXIT

"${CC:-cc}" -std=c99 -Wall -Wextra -Werror \
	-I"${repo_root}/source" -I"${repo_root}/source/ff15" -I"${repo_root}/include" \
	"${repo_root}/tests/psram-load-policy-host.c" -o "$test_bin"
"$test_bin"
