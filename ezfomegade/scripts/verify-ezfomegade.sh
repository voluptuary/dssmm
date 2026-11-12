#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"
python3 tests/test-migoomba-payload.py
make
bash scripts/run-host-test-suite.sh
node tests/browser-installer.mjs
for file in docs/manager/js/*.js; do node --check "$file"; done
bash scripts/test-launcher-iwram-placement.sh
python3 tests/test-cppcheck-input.py
