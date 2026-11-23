#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "$0")/.." && pwd)"
board="${1:-sd}"
case "$board" in
  sd|lite|chis) ;;
  *) echo "Unknown SuperFW board '$board' (choose sd, lite, or chis)." >&2; exit 2 ;;
esac
compression_ratio="${COMPRESSION_RATIO:-4}"
cd "$project_root"
build_number="$(git rev-list --count HEAD)"
make clean
make DSSMM_BUILD_NUMBER="$build_number" BOARD="$board" COMPRESSION_RATIO="$compression_ratio"
python3 scripts/test-boot-resources-arm.py
python3 scripts/check-carousel-memory.py firmware.ewram.elf
python3 scripts/test-native-settings-arm.py
python3 scripts/test-native-plugins-arm.py
python3 scripts/test-native-plugins-arm.py --native-gbc
python3 scripts/check-ingame-package.py
if [[ "${DSSMM_ARM_IGM:-0}" == 1 ]]; then
  python3 scripts/check-ingame-package.py --arm
fi
mkdir -p "build/$board"
cp superfw.gba "build/$board/superfw.fw"
python3 - "$board" "$build_number" <<'PY'
import hashlib,json,pathlib,subprocess,sys
board,number=sys.argv[1:]
sys.path.insert(0,str(pathlib.Path('scripts').resolve()))
from firmware_limits import check_size
def git(*args): return subprocess.check_output(['git',*args],text=True).strip()
image=pathlib.Path('build')/board/'superfw.fw'
fingerprint=hashlib.sha256()
for name in sorted(set(git('ls-files','--cached','--others','--exclude-standard','.').splitlines())):
    path=pathlib.Path(name)
    if path.is_file():
        fingerprint.update(name.encode()+b'\0'+hashlib.sha256(path.read_bytes()).digest())
record={'source_tree_sha256':fingerprint.hexdigest(),'board':board,'dssmm_build':int(number),'source_commit':git('rev-parse','HEAD'),
        'source_dirty':bool(git('status','--porcelain','--untracked-files=normal','--','.')),
        'firmware_sha256':hashlib.sha256(image.read_bytes()).hexdigest(),
        'validation':'built; runtime and hardware validation not implied'}
record.update(check_size(board,image.stat().st_size))
(image.parent/'build-info.json').write_text(json.dumps(record,indent=2)+'\n')
print(f"DSSMM Build {number} ({board}); source {record['source_commit']}; dirty={record['source_dirty']}")
PY
printf 'Built %s\n'  "$project_root/build/$board/superfw.fw"
