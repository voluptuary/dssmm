#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "$0")/.." && pwd)"
board="${1:-sd}"
case "$board" in
  sd|lite|chis) ;;
  *) echo "Unknown SuperFW board '$board' (choose sd, lite, or chis)." >&2; exit 2 ;;
esac
compression_ratio="${COMPRESSION_RATIO:-10}"
cd "$project_root"
make clean
make BOARD="$board" COMPRESSION_RATIO="$compression_ratio"
mkdir -p "build/$board"
cp superfw.gba "build/$board/superfw.fw"
printf 'Built %s\n' "$project_root/build/$board/superfw.fw"
