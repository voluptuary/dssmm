#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
"${CC:-cc}" -std=c99 -O1 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie \
    -I"$root/source" "$root/tests/plugin-policy-host.c" -o "$work/policy"
"$work/policy"
python3 "$root/tests/test-kernel-boot-host.py"
