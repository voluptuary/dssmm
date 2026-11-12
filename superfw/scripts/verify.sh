#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "$0")/.." && pwd)"
for tool in arm-none-eabi-gcc arm-none-eabi-g++ gcc g++ lcov; do
  command -v "$tool" >/dev/null || { echo "Missing required validation tool: $tool" >&2; exit 2; }
done
bash "$project_root/scripts/test-theme-settings.sh"
bash "$project_root/scripts/test-artwork.sh"
for board in sd lite chis; do
  bash "$project_root/scripts/build.sh" "$board"
done
make -C "$project_root/tests" clean
make -C "$project_root/tests" all
