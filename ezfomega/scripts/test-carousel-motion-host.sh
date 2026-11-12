#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
test_tmp=$(mktemp -d)
trap 'rm -rf "$test_tmp"' EXIT HUP INT TERM

awk '
    /^#define LAUNCHER_MOTION_CODE_(ADDRESS|END) / { print }
    /^static u32 Launcher_(MotionInstallCode|DrawMotionRest)\(/ { p=1 }
    p { print }
    p && /^}/ { p=0; found++ }
    END { if(found != 2) exit 1 }
' "$repo_root/source/carousel_motion.h" > "$test_tmp/motion.inc"

gcc -std=c99 -O2 -Wall -Wextra -Werror \
    -DMOTION_FILE="\"$test_tmp/motion.inc\"" \
    "$repo_root/tests/carousel-motion-host.c" -o "$test_tmp/motion"
"$test_tmp/motion"
