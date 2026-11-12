#!/usr/bin/env python3
"""Optional interoperability check against a supplied DSSCC checkout (no downloads).

Uses the real writer function with a tiny image adapter, avoiding Pillow/GUI deps.
Run: python3 tests/test-dsscc-writer.py /path/to/dsscc
"""
import ast
import ctypes as c
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
from types import SimpleNamespace

root = Path(__file__).resolve().parents[1]
source = Path(sys.argv[1]) / 'ezflash_gba_thumbs.py'
module = ast.parse(source.read_text())
writer = next(n for n in module.body if isinstance(n, ast.FunctionDef) and n.name == 'write_ezflash_bmp')
namespace = {'Image': SimpleNamespace(Image=object), 'Path': Path, 'struct': struct,
             'TRANSPARENT_THUMBNAIL_MARKER': b'DST1'}
exec(compile(ast.Module(body=[writer], type_ignores=[]), str(source), 'exec'), namespace)

class Image:
    def __init__(self, width): self.size = (width, 80)
    def getpixel(self, point):
        return [(0, 0, 0, 0), (255, 0, 0, 255), (0, 255, 0, 255),
                (0, 0, 255, 255), (0, 0, 0, 255)][point[0] % 5]

class Info(c.Structure):
    _fields_ = [('width', c.c_uint16), ('height', c.c_uint16), ('transparent', c.c_bool)]

with tempfile.TemporaryDirectory() as temp:
    temp = Path(temp)
    subprocess.run(['gcc', '-shared', '-fPIC', '-I', str(root/'src'),
                    str(root/'src/artwork.c'), '-o', str(temp/'artwork.so')], check=True)
    lib = c.CDLL(str(temp/'artwork.so'))
    reader_type = c.CFUNCTYPE(c.c_bool, c.c_void_p, c.c_uint32, c.c_void_p, c.c_size_t)
    lib.artwork_decode.argtypes = [reader_type, c.c_void_p, c.c_uint32,
                                   c.POINTER(c.c_uint8), c.c_size_t, c.POINTER(Info)]
    lib.artwork_decode.restype = c.c_bool
    for width in (80, 120):
        for marked in (False, True):
            dest = temp/'generated.bmp'
            namespace['write_ezflash_bmp'](Image(width), dest, mark_transparency=marked)
            data = dest.read_bytes()
            @reader_type
            def read(_ctx, off, out, size):
                if off + size > len(data): return False
                c.memmove(out, data[off:off+size], size)
                return True
            pixels = (c.c_uint8 * (width*80))()
            info = Info()
            assert lib.artwork_decode(read, None, len(data), pixels, len(pixels), c.byref(info))
            assert (info.width, info.height, info.transparent) == (width, 80, marked)
            assert list(pixels[:5]) == [0 if marked else 20, 25, 50, 200, 20]
print('Real DSSCC writer interoperability passed: both sizes, marked/unmarked and RGB channels.')
