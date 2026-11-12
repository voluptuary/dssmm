#!/usr/bin/env python3
"""Check the pinned image and exercise the actual GB/GBC loader branch with canaries."""
from pathlib import Path
import json
import os
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
meta = json.loads((ROOT / 'docs/plugins/migoomba.json').read_text())
subprocess.run(['python3', str(ROOT / 'scripts/embed-migoomba.py'), '--check'], check=True)
source = (ROOT / meta['kernel_source']).read_text()
definition = re.search(r'u32\s+(?:IWRAM_CODE\s+)?LoadEMU2PSRAM\([^;{}]*\)\s*\{', source)
if not definition:
    raise SystemExit('Missing platform emulator loader definition')
start = source.index('switch(is_EMU)', definition.end())
case = source[source.index('case 1:', start):source.index('default:', start)]
prelude = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "migoomba.h"
typedef uint32_t u32;
#define MAX_pReadCache_size 0x20000u
#define PSRAM_CAPACITY_BYTES 0x02000000u
#define KERNEL_PLUGIN_ROM_LIMIT 0x02000000u
static int fits(u32 offset, u32 size) {return offset <= PSRAM_CAPACITY_BYTES && size <= PSRAM_CAPACITY_BYTES - offset;}
#define PSRAM_LoadRangeFits fits
#define KernelPluginRangeFits fits
static unsigned char cache[MAX_pReadCache_size + 32], psram[MAX_pReadCache_size + 32];
static unsigned char *pReadCache = cache + 16, *PSRAMBase_S98 = psram + 16;
static unsigned copies, payload_size;
static const unsigned real_size = sizeof(migoomba_gba);
#define migoomba_gba_size payload_size
static void dmaCopy(const void *src, void *dst, unsigned bytes) {
    assert(bytes <= MAX_pReadCache_size); ++copies; memcpy(dst, src, bytes);
}
static u32 load(unsigned is_EMU) {
    u32 rom_start_address = 0;
    switch(is_EMU) {
'''
postlude = r'''
    default: return 0;
    }
    return rom_start_address;
load_failed:
    return 0;
}
int main(void) {
    assert((uintptr_t)migoomba_gba % 4 == 0 && real_size % 4 == 0);
    for(unsigned kind = 1; kind <= 2; ++kind) {
        memset(cache, 0xCC, sizeof(cache)); memset(psram, 0xCC, sizeof(psram));
        payload_size = real_size; copies = 0;
        assert(load(kind) == real_size && copies == 2);
        assert(!memcmp(pReadCache, migoomba_gba, real_size));
        assert(!memcmp(PSRAMBase_S98, migoomba_gba, real_size));
        for(unsigned i = 0; i < 16; ++i) assert(cache[i] == 0xCC && psram[i] == 0xCC);
        for(unsigned i = 16 + real_size; i < sizeof(cache); ++i) assert(cache[i] == 0xCC && psram[i] == 0xCC);
        for(unsigned fault = 0; fault < 2; ++fault) {
            payload_size = fault ? MAX_pReadCache_size + 4 : 0; copies = 0;
            assert(!load(kind) && !copies);
        }
    }
    puts("Actual GB/GBC embedded loader: exact payload/ROM offset, DMA alignment, canaries and invalid sizes passed.");
}
'''
with tempfile.TemporaryDirectory(prefix='migoomba-loader-') as folder:
    work = Path(folder)
    (work / 'test.c').write_text(prelude + case + postlude)
    subprocess.run([os.environ.get('CC', 'cc'), '-std=c11', '-O1', '-Wall', '-Wextra', '-Werror',
                    '-fsanitize=address,undefined', '-fno-pie', '-no-pie', '-I', str((ROOT / meta['embedded_header']).parent),
                    str(work / 'test.c'), '-o', str(work / 'test')], check=True)
    subprocess.run([str(work / 'test')], check=True)
