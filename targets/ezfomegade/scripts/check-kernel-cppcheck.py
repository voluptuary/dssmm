#!/usr/bin/env python3
"""Analyze real ARM-preprocessed C without duplicating bulk immutable assets in ASTs.

Only literal numeric const arrays of at least256 elements become opaque extern
arrays with identical types/dimensions. Mutable arrays and all function bodies
are retained. Payload identity/decompression is covered by separate asset tests.
"""
from pathlib import Path
import argparse
import os
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / ('targets/blue/source' if (ROOT/'targets/blue/source/ezbluekernel.c').exists() else 'source')
DEVKIT = Path(os.environ.get('DEVKITARM', '/opt/devkitpro/devkitARM'))
LIBGBA = Path(os.environ.get('LIBGBA', '/opt/devkitpro/libgba'))
# Match numeric payloads only, never code, strings or runtime initializers.
ARRAY = re.compile(
    rb'(?m)^[ \t]*(?P<decl>(?:static\s+)?const\s+[^;\n{}=]*?'
    rb'\b(?P<name>\w+)\[(?P<size>[^\]\n]*)\][^;{}=]*)\s*=\s*'
    rb'\{(?P<values>[^{}]*)\};')


def compact_assets(data):
    replaced = []
    def replace(match):
        numeric = re.sub(rb'/\*.*?\*/|//[^\n]*', b'', match['values'], flags=re.S)
        values = [v.strip() for v in numeric.split(b',') if v.strip()]
        if len(values) < 256:
            return match[0]
        if not all(re.fullmatch(rb'(?:0[xX][0-9a-fA-F]+|[0-9]+)[uUlL]*', v) for v in values):
            return match[0]
        declaration = match['decl']
        if b'volatile' in declaration.split():
            return match[0]
        if not match['size'].strip():
            declaration = declaration.replace(b'[]', b'[' + str(len(values)).encode() + b']', 1)
        declaration = re.sub(rb'^static\s+', b'', declaration)
        declaration = b'extern ' + declaration + b';'
        replaced.append(match['name'].decode('ascii'))
        return declaration + b'\n' * (match[0].count(b'\n') - declaration.count(b'\n'))
    return ARRAY.sub(replace, data), replaced


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('files', nargs='*', help='Paths relative to the active source directory; default all C units')
    args = parser.parse_args()
    units = [SOURCE/name for name in args.files] if args.files else sorted(SOURCE.rglob('*.c'))
    with tempfile.TemporaryDirectory(prefix='kernel-cppcheck-') as folder:
        for source in units:
            command = [str(DEVKIT/'bin/arm-none-eabi-gcc'), '-E', '-CC', '-mcpu=arm7tdmi',
                       '-mthumb', '-mthumb-interwork', '-D__USES_INITFINI__',
                       '-I'+str(LIBGBA/'include'), '-iquote'+str(ROOT/'include'),
                       '-iquote'+str(SOURCE/'ff15'), '-iquote'+str(ROOT/'images'), str(source)]
            preprocessed = subprocess.check_output(command)
            compacted, arrays = compact_assets(preprocessed)
            # Cppcheck2.10 needs standard directives for accurate nested-header locations.
            compacted = re.sub(rb'(?m)^# (\d+) ("[^"\n]+")([^\n]*)$', rb'#line \1 \2', compacted)
            path = Path(folder)/'unit.c'
            path.write_bytes(compacted)
            print(f'ARM Cppcheck: {source.relative_to(ROOT)}; {len(arrays)} bulk immutable arrays kept opaque.', flush=True)
            subprocess.run(['cppcheck', '--enable=warning,performance,portability',
                            '--max-configs=1', '--platform=unix32', '--language=c',
                            '--suppress=missingIncludeSystem', '--suppress=arithOperationsOnVoidPointer',
                            '--inline-suppr', '--template=gcc', str(path)], check=True)
    print('ARM-preprocessed active C analysis completed. Inspect diagnostics; no hardware qualification.')


if __name__ == "__main__":
    main()
