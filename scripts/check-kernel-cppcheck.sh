#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
python3 "$root/tests/test-cppcheck-input.py"
python3 "$root/scripts/check-kernel-cppcheck.py" "$@"
