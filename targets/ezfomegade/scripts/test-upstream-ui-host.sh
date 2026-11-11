#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
task_dir=$(mktemp -d)
trap 'rm -rf "$task_dir"' EXIT
python3 - "$task_dir" <<'PY'
import pathlib, re, sys
s = pathlib.Path('source/ezkernelnew.c').read_text()
names = ['Launcher_ReadWrapAroundSetting']
out = []
for name in names:
    m = re.search(r'^static [^\n]+ '+name+r'\([^\n]*\)\n\{', s, re.M)
    assert m, name
    end = s.index('\n}', m.end()) + 2
    out.append(s[m.start():end])
pathlib.Path(sys.argv[1], 'readers.inc').write_text('\n'.join(out))
# Preservation checks against the actual pre-integration fork, not upstream.
import subprocess
baseline = subprocess.check_output(['git','show','1cca2d6:source/ezkernelnew.c'],text=True)
for name in ['Launcher_StartGetLastTitle','Launcher_SplitStartTitle','Launcher_StartScrollTextLine','Launcher_StartTitleShouldScroll']:
    def body(text):
        m = re.search(r'^static [^\n]+ '+name+r'\([^\n]*\)\n\{',text,re.M)
        assert m, name
        return text[m.start():text.index('\n}',m.end())+2]
    assert body(s) == body(baseline), name
assert 'Launcher_ReadRememberCheatsSetting();' in s
assert 'Launcher_ReadCleanListSetting();' in s
print('PASS: current Last played title functions preserved against fork baseline')
PY
cc -std=c99 -Wall -Wextra -Werror -I source -I "$task_dir" tests/upstream-ui-host.c -o "$task_dir/test"
"$task_dir/test"
