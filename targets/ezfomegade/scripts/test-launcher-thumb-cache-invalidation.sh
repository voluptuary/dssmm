#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/.." && pwd)"
test_tmp="$(mktemp -d)"
trap 'rm -rf "$test_tmp"' EXIT

awk '
	/^static void Launcher_InvalidateThumbCacheEntries\(/ && $0 !~ /;[ \\t]*$/ { capture = 1 }
	capture { print }
	capture && /^}/ { found++; exit }
	END { if(found != 1) exit 1 }
' "$repo_root/source/ezkernelnew.c" > "$test_tmp/cache-invalidation.inc"
[ -s "$test_tmp/cache-invalidation.inc" ] || {
	echo 'FAIL: could not extract thumbnail cache invalidation helper' >&2
	exit 1
}

python3 - "$repo_root/source/ezkernelnew.c" <<'PY'
from pathlib import Path
import sys

source = Path(sys.argv[1]).read_text()
start = source.find("static void Launcher_CycleViewModeAndRedraw(")
if start < 0:
    raise SystemExit("FAIL: view-mode transition function is missing")
end = source.find("\n}\n", start)
if end < 0:
    raise SystemExit("FAIL: view-mode transition function did not terminate")
function = source[start:end]
steps = [
    "new_view = Launcher_ActiveViewMode();",
    "if((new_view == LAUNCHER_VIEW_LIST) || (new_view == LAUNCHER_VIEW_LIST_ART))",
    "Launcher_InvalidateThumbCacheEntries();",
    "Launcher_ActivateThumbnailWorkspace(LAUNCHER_VIEW_LIST_ART);",
]
positions = [function.find(step) for step in steps]
if any(position < 0 for position in positions) or positions != sorted(positions):
    raise SystemExit("FAIL: list-like transition must invalidate metadata before list-art redraw")

scratch = "#define LAUNCHER_LIST_SELECTED_ROW_SCRATCH 0x14C00"
previous_slot = "launcher_cache_prev.thumb_data = pReadCache + 0x14C36;"
if scratch not in source or previous_slot not in source:
    raise SystemExit("FAIL: expected row-scratch/previous-image overlap is missing")
PY

gcc -std=c99 -O2 -Wall -Wextra -Werror \
	-DCACHE_INVALIDATION_FILE="\"$test_tmp/cache-invalidation.inc\"" \
	"$repo_root/tests/launcher-thumb-cache-invalidation-host.c" \
	-o "$test_tmp/launcher-thumb-cache-invalidation-host"
"$test_tmp/launcher-thumb-cache-invalidation-host"
