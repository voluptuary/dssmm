#!/bin/sh
# Exercises the Makefile's 2 MiB kernel-size guard without devkitPro.
# The "$(OUTPUT).gba" rule is extracted verbatim from the Makefile and run through GNU make with stub
# objcopy/gbafix, so the real recipe text is what is tested.
# Usage: scripts/test-kernel-size-guard-host.sh [git-ref]   (default: the working-tree Makefile)
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
test_tmp=$(mktemp -d)
trap 'rm -rf "$test_tmp"' EXIT HUP INT TERM

if [ "$#" -ge 1 ]; then
	git -C "$repo_root" show "$1:Makefile" > "$test_tmp/repo.mk"
else
	cp "$repo_root/Makefile" "$test_tmp/repo.mk"
fi

limit=$(sed -n 's/^KERNEL_SIZE_LIMIT[[:space:]]*:=[[:space:]]*\([0-9][0-9]*\).*/\1/p' "$test_tmp/repo.mk")
[ -n "$limit" ] || { echo "FAIL: KERNEL_SIZE_LIMIT not found in Makefile" >&2; exit 1; }

# From the .gba rule header up to (not including) the .elf rule header.
awk '/^\$\(OUTPUT\)\.gba[ \t]*:/{p=1} /^\$\(OUTPUT\)\.elf[ \t]*:/{p=0} p' "$test_tmp/repo.mk" > "$test_tmp/rule.mk"
[ -s "$test_tmp/rule.mk" ] || { echo "FAIL: could not extract the .gba rule" >&2; exit 1; }

mkdir "$test_tmp/bin" "$test_tmp/work"
printf '#!/bin/sh\ncp "$3" "$4"\n' > "$test_tmp/bin/objcopy-stub"   # objcopy -O binary IN OUT
printf '#!/bin/sh\nexit 0\n' > "$test_tmp/bin/gbafix"
chmod +x "$test_tmp/bin/objcopy-stub" "$test_tmp/bin/gbafix"
{
	printf 'OUTPUT := out\nOBJCOPY := objcopy-stub\nSILENTCMD := @\n'
	printf 'GAME_TITLE := T\nGAME_CODE := C\nMAKER_CODE := M\nKERNEL_SIZE_LIMIT := %s\n\n' "$limit"
	cat "$test_tmp/rule.mk"
} > "$test_tmp/work/Makefile"

PATH="$test_tmp/bin:$PATH"
cd "$test_tmp/work"

fail=0
check() { if [ "$1" -eq 0 ]; then echo "  ok    $2"; else echo "  FAIL  $2"; fail=1; fi; }
build() { rm -f out.gba; head -c "$1" /dev/zero > out.elf; if make >build.log 2>&1; then return 0; else return 1; fi; }

echo "[1] image exactly at the limit ($limit bytes) is accepted"
if build "$limit"; then check 0 "make succeeds"; else check 1 "make succeeds"; fi
[ "$(wc -c < out.gba)" -eq "$limit" ] && check 0 "out.gba kept at the limit" || check 1 "out.gba kept at the limit"

echo "[2] image one byte over the limit is rejected"
if build "$((limit + 1))"; then check 1 "make fails"; else check 0 "make fails"; fi
grep -q "exceeds" build.log && check 0 "message names the limit" || check 1 "message names the limit"
[ ! -e out.gba ] && check 0 "no oversized out.gba left behind" || check 1 "no oversized out.gba left behind"

echo "[3] re-running make after a rejected build must not silently succeed"
if make >rerun.log 2>&1; then check 1 "second make still fails"; else check 0 "second make still fails"; fi
[ ! -e out.gba ] && check 0 "still no out.gba after the re-run" || check 1 "still no out.gba after the re-run"

echo "[4] a comfortably small image is accepted"
if build "$((limit - 30032))"; then check 0 "make succeeds"; else check 1 "make succeeds"; fi
grep -q "headroom\|below\|bytes" build.log && check 0 "build reports the size" || check 1 "build reports the size"

if [ "$fail" -eq 0 ]; then echo "Kernel size guard host test passed."; else echo "Kernel size guard host test FAILED." >&2; exit 1; fi
