#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "$0")/.." && pwd)"
for tool in arm-none-eabi-gcc arm-none-eabi-g++ gcc g++ lcov; do
  command -v "$tool" >/dev/null || { echo "Missing required validation tool: $tool" >&2; exit 2; }
done
bash "$project_root/scripts/test-theme-settings.sh"
bash "$project_root/scripts/test-settings-io.sh"
bash "$project_root/scripts/test-clean-list.sh"
bash "$project_root/scripts/test-recents.sh"
bash "$project_root/scripts/test-fileutil-boundaries.sh"
bash "$project_root/scripts/test-patchengine-bounds.sh"
bash "$project_root/scripts/test-nds-loader.sh"
bash "$project_root/scripts/test-loader-layout.sh"
bash "$project_root/scripts/test-nor-launch.sh"
bash "$project_root/scripts/test-pogo-plugins.sh"
python3 "$project_root/scripts/test-pogo-adapters.py"
python3 "$project_root/scripts/test-pogo-reset-arm.py"
python3 "$project_root/scripts/test-pogo-return-arm.py"
python3 "$project_root/scripts/test-pogo-boot-arm.py"
python3 "$project_root/scripts/test-icon-resources-arm.py"
bash "$project_root/scripts/test-emulator-recovery.sh"
bash "$project_root/scripts/test-gamegear-icon.sh"
bash "$project_root/scripts/test-start-corners.sh"
bash "$project_root/scripts/test-topbar.sh"
bash "$project_root/scripts/test-settings-highlight.sh"
bash "$project_root/scripts/test-update-unlock.sh"
bash "$project_root/scripts/test-system-layout.sh"
python3 "$project_root/tests/test-firmware-limits.py"
bash "$project_root/scripts/test-letter-folder-pack.sh"
bash "$project_root/scripts/test-dssmm-font.sh"
bash "$project_root/scripts/test-dssmm-background.sh"
bash "$project_root/scripts/test-dssmm-panel.sh"
python3 "$project_root/tests/test-preview-pixels.py"
bash "$project_root/scripts/test-launcher-navigation.sh"
bash "$project_root/scripts/test-launcher-profile.sh"
bash "$project_root/scripts/test-artwork.sh"
bash "$project_root/scripts/test-ingame-selection.sh"
bash "$project_root/scripts/test-browser-motion.sh"
bash "$project_root/scripts/test-carousel-cache.sh"
bash "$project_root/scripts/test-fast-carousel-pages.sh"
bash "$project_root/scripts/test-browser-navigation.sh"
for board in sd lite chis; do
  bash "$project_root/scripts/build.sh" "$board"
done
bash "$project_root/scripts/test-native-settings-mgba.sh"
bash "$project_root/scripts/test-native-menu-visuals-mgba.sh"
DSSMM_PREVIEW_THEME=7 DSSMM_PREVIEW_TOPBAR=5 DSSMM_PREVIEW_SCENES="1 2 15" bash "$project_root/scripts/test-native-menu-visuals-mgba.sh"
DSSMM_PREVIEW_THEME=6 DSSMM_PREVIEW_TOPBAR=0 DSSMM_PREVIEW_SCENES=15 bash "$project_root/scripts/test-native-menu-visuals-mgba.sh"
bash "$project_root/scripts/test-start-caption-mgba.sh"
bash "$project_root/scripts/test-plugin-preview-mgba.sh"
python3 "$project_root/tools/pack-dssmm-resources.py" --check
make -C "$project_root/tests" clean
make -C "$project_root/tests" all
