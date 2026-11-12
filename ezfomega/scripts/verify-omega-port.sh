#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"
omega_test_dir="$(mktemp -d)"
trap 'rm -rf "$omega_test_dir"' EXIT
# Keep target validation separate from the inherited DE audit wrapper.
bash scripts/test-cheat-state-host.sh
bash scripts/test-cheat-row-host.sh
bash scripts/test-savefile-host.sh
bash scripts/test-omega-patch-host.sh
bash scripts/test-nes-patch-host.sh
bash scripts/test-patch-write-host.sh
bash scripts/test-marked-artwork-host.sh
bash scripts/test-oem-path-host.sh
bash scripts/test-save-load-host.sh
bash scripts/test-omega-trim-host.sh
bash scripts/test-omega-nor-scan-host.sh
bash scripts/test-omega-nor-load-host.sh
bash scripts/test-omega-nor-progress-host.sh
bash scripts/test-nor-background-host.sh
python3 tests/test-omega-splash.py
bash scripts/test-psram-load-policy.sh
bash scripts/test-launcher-settings-readers-host.sh
bash scripts/test-launcher-wrap-system-host.sh
bash scripts/test-launcher-clean-list-cache-host.sh
"${CC:-cc}" -std=c99 -O1 -Wall -Wextra -Werror -Isource tests/omega/languages.c -o "$omega_test_dir/languages"
"$omega_test_dir/languages"
bash scripts/test-launcher-enum-settings-host.sh
bash scripts/test-launcher-art-style-policy-host.sh
bash scripts/test-carousel-motion-host.sh
bash scripts/test-custom-manifest-host.sh
bash scripts/test-launcher-thumb-cache-invalidation.sh
bash scripts/test-carousel-lookup-scaling-host.sh
python3 tests/test-omega-index.py
python3 tests/test-omega-launch-copy.py
python3 tests/test-omega-nor-artwork.py
bash scripts/test-fatfs-long-name-host.sh
bash scripts/test-omega-artwork-host.sh
bash scripts/test-omega-transparency-host.sh
make BUILD=build-omega TARGET=omega-port
bash scripts/test-omega-layout.sh
python3 tests/test-omega-plugin-policy.py
python3 - <<'PY'
from pathlib import Path
size=Path('omega-port.gba').stat().st_size
assert 0 < size <= 2097152
print(f'Original Omega development image: {size} bytes, {2097152-size} bytes headroom; hardware untested.')
PY
