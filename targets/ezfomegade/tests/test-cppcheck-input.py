#!/usr/bin/env python3
"""Guard Cppcheck input fidelity: retain code/mutable data and declaration spans."""
from pathlib import Path
import runpy
root=Path(__file__).resolve().parents[1]
compact=runpy.run_path(str(root/'scripts/check-kernel-cppcheck.py'))['compact_assets']
values=b',\n'.join(str(i).encode() for i in range(256))
source=b'const unsigned short asset[]\n__attribute__((aligned(4))) = {\n'+values+b'\n};\nint function(void) {return 9;}\n'
result,names=compact(source)
assert names==['asset'] and b'extern const unsigned short asset[256]' in result
assert result.count(b'\n')==source.count(b'\n')
assert result.endswith(b'int function(void) {return 9;}\n')
for declaration in [b'unsigned short mutable[256]',b'const volatile unsigned short hardware[256]']:
    source=declaration+b'={'+values+b'};\n'
    assert compact(source)==(source,[])
for source in [b'const unsigned short small[]={1,2,3};\n', b'const char *names[]={"text", "text"};\n']:
    assert compact(source)==(source,[])
source=b'static const unsigned short fixed[256] = {'+values+b'};\n'
result,names=compact(source)
assert b'extern const unsigned short fixed[256]' in result and b'static' not in result
assert result.count(b'\n')==source.count(b'\n')
print('Cppcheck input: immutable array dimensions/types/lines retained; code, mutable and volatile data untouched.')

source=b'    static const unsigned short table[] = { /* lookup */\n'+values+b' /* end */\n};\n'
result,names=compact(source)
assert names==['table'] and b'extern const unsigned short table[256]' in result
assert result.count(b'\n')==source.count(b'\n')
