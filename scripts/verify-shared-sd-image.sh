#!/usr/bin/env bash
set -euo pipefail

# Run six source-matched card profiles against identical initial SD bytes.
root="$(cd "$(dirname "$0")/.." && pwd)"
image="${1:?usage: verify-shared-sd-image.sh pristine-image runner-directory [results-directory]}"
runners="${2:?provide the directory containing the pogo.gba mGBA card runners}"
results="${3:-$root/build/shared-sd-image}"
mkdir -p "$results"
image="$(realpath "$image")"
runners="$(realpath "$runners")"
baseline="$(sha256sum "$image" | cut -d' ' -f1)"
filesystem="$(blkid -o value -s TYPE "$image")"
case "$filesystem" in
    vfat) check_image() { fsck.vfat -n "$1" >/dev/null; } ;;
    exfat) check_image() { fsck.exfat -n "$1" >/dev/null; } ;;
    *) echo "Unsupported SD image filesystem: $filesystem" >&2; exit 2 ;;
esac
check_image "$image"

run_profile() {
    local name="$1" board="$2" kernel="$3" runner="$4" events="$5"
    local copy="$results/$name.img" log="$results/$name.log"
    local menu_copy="$results/$name-menu.img"
    test -s "$kernel"
    cp --reflink=auto "$image" "$menu_copy"
    test "$(sha256sum "$menu_copy" | cut -d' ' -f1)" = "$baseline"
    cp --reflink=auto "$image" "$copy"
    test "$(sha256sum "$copy" | cut -d' ' -f1)" = "$baseline"
    check_image "$copy"
    case "$runner" in
        ez)
            "$runners/ezflash-mgba-runner" "$kernel" 1000 "$menu_copy" \
                "$results/$name-menu.ppm" >"$results/$name-menu.log"
            "$runners/ezflash-mgba-runner" "$kernel" 3200 "$copy" "$results/$name.ppm" "$events" >"$log"
            rg -q 'failures=0' "$log"
            rg -q 'psram_reads=[1-9][0-9]*([[:space:]]|$)' "$log"
            ;;
        supercard)
            A26_SUPERCARD_BOARD="$board" "$runners/supercard-mgba-runner" \
                "$kernel" 1000 "$menu_copy" "$results/$name-menu.ppm" >"$results/$name-menu.log"
            A26_SUPERCARD_BOARD="$board" "$runners/supercard-mgba-runner" \
                "$kernel" 3200 "$copy" "$results/$name.ppm" "$events" >"$log"
            rg -q 'load_failures=0' "$log"
            rg -q 'region=cartridge' "$log"
            ;;
    esac
    check_image "$menu_copy"
    check_image "$copy"
    python3 - "$results/$name-menu.ppm" <<'PY'
from PIL import Image
import sys

image = Image.open(sys.argv[1]).convert("RGB")
if image.size != (240, 160) or len(set(image.getdata())) < 2:
    raise SystemExit("missing visible 240x160 menu capture: " + sys.argv[1])
PY
    printf '%s baseline=%s result=%s\n' "$name" "$baseline" "$(sha256sum "$copy" | cut -d' ' -f1)"
}

ez_events='350-360:0x80,380-390:0x1,450-470:0x1,550-580:0x1'
sc_events='600:0x1,1200:0x1,1800:0x80,1900:0x1,2200-2210:0x1'
run_profile ezfomegade ezfomegade "$root/ezfomegade/ezfomegade.gba" ez "$ez_events"
run_profile ezfomega ezfomega "$root/ezfomega/omega-port.gba" ez "$ez_events"
run_profile ezfblue ezfblue "$root/ezfblue/targets/blue/ezbluekernel-development.gba" ez "$ez_events"
for board in sd lite chis; do
    run_profile "supercard-$board" "$board" "$root/superfw/build/$board/superfw.fw" supercard "$sc_events"
done
test "$(sha256sum "$image" | cut -d' ' -f1)" = "$baseline"
