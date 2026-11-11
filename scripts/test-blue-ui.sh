#!/usr/bin/env bash
# Run shared software/UI tests against BLUE's actual sources, never the retained donor.
set -euo pipefail
repo_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
test_root="$(mktemp -d)"
trap 'rm -rf "$test_root"' EXIT
mkdir "$test_root/source" "$test_root/scripts"
# Real copies are essential: filesystem tests instrument their private ff15 tree.
cp -R "$repo_dir/targets/blue/source/." "$test_root/source/"
cp "$test_root/source/ezbluekernel.c" "$test_root/source/ezkernelnew.c"
# Fixture-relative ../source includes must also resolve to the private BLUE source.
cp -R "$repo_dir/tests" "$test_root/tests"
for item in images docs files; do ln -s "$repo_dir/$item" "$test_root/$item"; done
ln -s "$(git -C "$repo_dir" rev-parse --absolute-git-dir)" "$test_root/.git"
tests=(
    cheat-state-host cheat-row-host fat-table-builder-host fatfs-long-name-host
    carousel-motion-host carousel-lookup-scaling-host custom-manifest-host
    launcher-art-border-host launcher-art-style-policy-host launcher-artwork-draw-host
    launcher-auto-theme-host launcher-clean-list-cache-host launcher-enum-settings-host
    launcher-favourite-toggle-host launcher-navigation-host launcher-settings-readers-host
    launcher-thumb-cache-invalidation launcher-wrap-system-host nor-thumbnail-fallback-host
    popup-button-hints-host psram-load-policy save-popup-artwork-host
    savefile-host upstream-ui-host
)
# Optional focused checks retain the complete list as the default verifier.
if (( $# )); then
    for requested in "$@"; do
        case " ${tests[*]} " in
            *" $requested "*) ;;
            *) echo "Unknown BLUE software suite: $requested" >&2; exit 2 ;;
        esac
    done
    tests=("$@")
fi
for test in "${tests[@]}"; do
    cp "$repo_dir/scripts/test-$test.sh" "$test_root/scripts/"
    if [[ "$test" == cheat-state-host ]]; then
        sed -i 's@tests/cheat-state-host.c@tests/blue-cheat-state-host.c@' "$test_root/scripts/test-$test.sh"
    fi
    if [[ "$test" == savefile-host ]]; then
        # RAM placement changed; extraction still tests the same production function.
        sed -i 's/\^u32 IWRAM_CODE Save_savefile/\^u32 (IWRAM_CODE )?Save_savefile/' "$test_root/scripts/test-$test.sh"
    fi
    echo "BLUE software suite: $test"
    bash "$test_root/scripts/test-$test.sh"
done
echo "All ${#tests[@]} BLUE software/UI suites passed against targets/blue/source."
