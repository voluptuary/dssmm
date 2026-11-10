#!/usr/bin/env bash
set -euo pipefail
repo_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"
# Keep target validation separate from the inherited DE audit wrapper.
bash scripts/test-cheat-state-host.sh
bash scripts/test-cheat-row-host.sh
bash scripts/test-savefile-host.sh
bash scripts/test-omega-nor-load-host.sh
bash scripts/test-psram-load-policy.sh
bash scripts/test-omega-artwork-host.sh
bash scripts/test-omega-transparency-host.sh
make BUILD=build-omega TARGET=omega-port
bash scripts/test-omega-layout.sh
python3 - <<'PY'
from pathlib import Path
size=Path('omega-port.gba').stat().st_size
assert 0 < size <= 2097152
print(f'Original Omega development image: {size} bytes, {2097152-size} bytes headroom; hardware untested.')
PY
