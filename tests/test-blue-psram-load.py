#!/usr/bin/env python3
"""Compile the actual BLUE loader with banked-memory and SD fault mocks."""

from pathlib import Path
import os
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
RELATIVE = "targets/blue/source/ezbluekernel.c"
if len(sys.argv) == 2:
    source = subprocess.check_output(["git", "show", f"{sys.argv[1]}:{RELATIVE}"], cwd=ROOT).decode("latin1")
else:
    source = (ROOT / RELATIVE).read_bytes().decode("latin1")
start = source.index("u32 IWRAM_CODE Loadfile2PSRAM(TCHAR *filename)")
end = source.index("//---------------------------------------------------------------", start)
function = source[start:end]

PREAMBLE = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ez_define.h"
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef volatile uint16_t vu16;
typedef unsigned UINT;
typedef char TCHAR;
typedef struct { uint64_t size; uint32_t cursor; } FIL;
#define IWRAM_CODE
#define FR_OK 0
#define FA_READ 1
#define f_size(file) ((file)->size)
static FIL gfile;
static u8 pReadCache[MAX_pReadCache_size];
static int gl_reset_on, gl_rts_on, gl_sleep_on, gl_cheat_on;
static unsigned reads, copies, closes, patches, seeks;
static unsigned fail_read, short_read, page;
static int fail_open, fail_seek, fail_close;
static u8 pattern(uint32_t offset) { return (u8)((offset >> 13) ^ offset ^ 0x5A); }
static int f_open(FIL *file, const char *name, int mode) {
    (void)name; (void)mode; file->cursor = 0; return fail_open;
}
static int f_lseek(FIL *file, uint32_t pos) {
    seeks++; file->cursor = pos; return fail_seek;
}
static int f_read(FIL *file, void *buffer, unsigned bytes, UINT *received) {
    assert(bytes <= MAX_pReadCache_size);
    reads++;
    if (reads == fail_read) { *received = 0; return 1; }
    unsigned count = bytes;
    if (file->size - file->cursor < count) count = file->size - file->cursor;
    if (reads == short_read && count) count--;
    for (unsigned i = 0; i < count; i++) ((u8 *)buffer)[i] = pattern(file->cursor + i);
    *received = count; file->cursor += count; return 0;
}
static int f_close(FIL *file) { (void)file; closes++; return fail_close; }
static void SetPSRampage(unsigned value) { page = value; }
#define Clear(...) ((void)0)
#define DrawHZText12(...) ((void)0)
/* Vendor progress formatting uses ARM long widths; it is outside this test. */
#define sprintf(buffer, ...) ((void)(buffer))
static void PatchInternal(u32 *data, unsigned bytes, unsigned offset) {
    assert((void *)data == pReadCache && bytes == MAX_pReadCache_size);
    assert(offset == patches * MAX_pReadCache_size); patches++;
}
static void dmaCopy(const void *src, void *dst, unsigned bytes) {
    uint32_t offset = copies * MAX_pReadCache_size;
    assert(src == pReadCache && bytes == MAX_pReadCache_size);
    assert(page == (offset / 0x800000) * 0x1000);
    assert((uintptr_t)dst == 0x08800000u + offset % 0x800000);
    assert(offset + bytes <= 0x4000000u);
    for (unsigned i = 0; i < bytes; i++)
        assert(((const u8 *)src)[i] == (offset + i < gfile.size ? pattern(offset + i) : 0xFF));
    copies++;
}
'''
MAIN = r'''
static void reset(uint64_t size) {
    gfile.size = size; gfile.cursor = 0;
    reads = copies = closes = patches = seeks = 0;
    fail_read = short_read = fail_open = fail_seek = fail_close = 0;
    gl_reset_on = gl_rts_on = gl_sleep_on = gl_cheat_on = 0;
    page = 0xFFFF; memset(pReadCache, 0x77, sizeof pReadCache);
}
int main(void) {
    uint64_t lengths[] = {1, 0x1FFFF, 0x20000, 0x20001, 0x7FFFFF, 0x800001, 0x4000000};
    for (unsigned i = 0; i < sizeof lengths / sizeof lengths[0]; i++) {
        reset(lengths[i]);
        assert(Loadfile2PSRAM("game.gba") == 0);
        assert(copies == (lengths[i] + 0x1FFFF) / 0x20000);
        assert(closes == 1 && page == 0 && seeks == 1);
    }
    uint64_t invalid[] = {0, 0x4000001, UINT64_C(0x100000001)};
    for (unsigned i = 0; i < sizeof invalid / sizeof invalid[0]; i++) {
        reset(invalid[i]); assert(Loadfile2PSRAM("game.gba") != 0);
        assert(!reads && !copies && closes == 1 && page == 0);
    }
    reset(1); fail_open = 1; assert(Loadfile2PSRAM("game.gba") != 0);
    assert(!reads && !closes && !copies && page == 0);
    reset(1); fail_seek = 1; assert(Loadfile2PSRAM("game.gba") != 0);
    assert(!reads && closes == 1 && !copies && page == 0);
    reset(1); fail_close = 1; assert(Loadfile2PSRAM("game.gba") != 0);
    assert(copies == 1 && closes == 1 && page == 0);
    for (unsigned nth = 1; nth <= 65; nth += 64) {
        reset(0x820000); fail_read = nth;
        assert(Loadfile2PSRAM("game.gba") != 0);
        assert(copies == nth - 1 && closes == 1 && page == 0);
        reset(0x820000); short_read = nth;
        assert(Loadfile2PSRAM("game.gba") != 0);
        assert(copies == nth - 1 && closes == 1 && page == 0);
    }
    reset(0x20001); gl_cheat_on = 1;
    assert(Loadfile2PSRAM("game.gba") == 0 && patches == 2);
    puts("BLUE native loader: exact I/O, padding, 64 MiB bounds and all 8 banks passed.");
    return 0;
}
'''

with tempfile.TemporaryDirectory() as folder:
    cfile = Path(folder) / "test.c"
    binary = Path(folder) / "test"
    cfile.write_text(PREAMBLE + function + MAIN)
    subprocess.run([os.environ.get("CC", "cc"), "-std=gnu99", "-O2", "-Wall", "-Wextra",
                    "-Werror", "-I", str(ROOT / "targets/blue/source"),
                    str(cfile), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)

if len(sys.argv) == 1:
    # Failure must precede patch-file creation, further patching and game boot.
    caller = source[source.index("res=Loadfile2PSRAM(pfilename);"):]
    assert caller.index("if (res != 0)") < caller.index("Make_pat_file")
    assert caller.index("return 0;") < caller.index("Make_pat_file")
