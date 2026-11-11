#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef BOUNDED_NES_API
#include <sanitizer/asan_interface.h>
#endif
typedef uint32_t u32;
typedef uint8_t u8;
#define IWRAM_CODE
static u32 is_Nes, Nes_index, Nes_index_17_patch, windows_offset;
#ifdef BOUNDED_NES_API
static u8 Nes_probe_state;
#endif
static u8 gba_nes_patch_bin[0x340];
static unsigned writes;
static u32 last_address, last_value;
static void Write(u32 address, const u8 *buffer, u32 size)
{
    assert(buffer && size);
    writes++;
    last_address = address;
    if(size == 4) memcpy(&last_value, buffer, 4);
}
#include "nes-patch.impl.h"
static void detect(u32 *data, u32 bytes)
{
#ifdef BOUNDED_NES_API
    CheckNes(data, bytes);
#else
    (void)bytes; CheckNes(data);
#endif
}
static bool patch(u32 *data, u32 bytes)
{
#ifdef BOUNDED_NES_API
    return PatchNes(data, bytes);
#else
    (void)bytes; return PatchNes(data);
#endif
}
static void valid(u32 *data, unsigned variant)
{
    static const u32 signature[] = {
        0xE28F503C,0xE8B503D3,0xE129F007,0xE281DEBA,0xE129F008,
        0xE281DEBE,0xE129F009,0xE281DC0B,0xE92D0003,0xEF110000,0xE8BD8001
    };
    memset(data, 0, 0x20000);
    data[0] = 0xEA000006;
    memcpy(data + 8, signature, sizeof signature);
    if(variant == 2) data[8] = 0xE28F5040;
    data[8 + 16 + variant] = 0x08000104;
    data[64] = 0x3032;
    windows_offset = writes = 0;
    detect(data, 0x20000);
    assert(is_Nes == variant);
}
int main(int argc, char **argv)
{
    assert(argc == 2);
    u32 *data = malloc(0x20000); assert(data);
    valid(data, 1);
    if(!strcmp(argv[1], "valid")) {
        assert(patch(data, 0x20000));
        assert(writes == 4 && last_address == 0xFF86C && last_value == 0x060000F8);
        valid(data, 2); assert(patch(data, 0x20000));
        assert(writes == 4 && last_address == 0xFF86C && last_value == 0x060000F8);
    } else if(!strcmp(argv[1], "branch")) {
        data[0] = 0xEA007FFE; detect(data, 0x20000); assert(!is_Nes);
    } else if(!strcmp(argv[1], "stale")) {
        data[0] = 0; detect(data, 0x20000); assert(!is_Nes);
    } else if(!strcmp(argv[1], "variant")) {
        data[8] = 0xE28F5020; detect(data, 0x20000); assert(!is_Nes);
    } else if(!strcmp(argv[1], "probe")) {
        data[25] = 0; assert(patch(data, 0x20000)); assert(writes == 3);
    } else if(!strcmp(argv[1], "later")) {
        assert(patch(data, 0x20000));
        memset(data, 0, 0x20000); windows_offset = 0x20000; writes = 0;
        assert(patch(data, 0x20000));
        assert(writes == 4 && last_address == 0xFF86C && last_value == 0x060000F8);
    } else if(!strcmp(argv[1], "short")) {
        detect(data, 36); assert(!is_Nes);
    } else if(!strcmp(argv[1], "misaligned")) {
        data[25] = 0x08000105; assert(patch(data, 0x20000)); assert(writes == 3);
    } else if(!strcmp(argv[1], "pending")) {
        data[25] = 0x08020004;
        assert(patch(data, 0x20000) && writes == 3);
        memset(data, 0, 0x20000); data[0] = 0x3032; windows_offset = 0x20000; writes = 0;
        assert(patch(data, 0x20000));
        assert(writes == 4 && last_address == 0xFF86C);
#ifdef BOUNDED_NES_API
    } else if(!strcmp(argv[1], "fuzz")) {
        assert(NesNativeBytes(-1) == 0 && NesNativeBytes(0) == 0);
        assert(NesNativeBytes(1) == 1 && NesNativeBytes(0x800001) == 0x800000);
        u32 seed = 1;
        for(unsigned iteration = 0; iteration < 5000; iteration++) {
            valid(data, iteration % 2 + 1);
            seed = seed * 1664525 + 1013904223;
            u32 available = 128 + (seed % (0x20000 - 128));
            available &= ~7u;
            data[0] = iteration % 4 ? (0xEA000000 | (seed & 0xFFFFFF)) : 0xEA000006;
            __asan_poison_memory_region((u8 *)data + available, 0x20000 - available);
            detect(data, available);
            __asan_unpoison_memory_region(data, 0x20000);
            valid(data, iteration % 2 + 1);
            seed = seed * 1664525 + 1013904223;
            windows_offset = (seed & 255) * 0x20000;
            u32 pointer = 0x08000004 + windows_offset + ((seed >> 8) & 0x3FFFC);
            if(iteration % 4 == 0) pointer = seed;
            data[24 + is_Nes] = pointer;
            if(windows_offset) Nes_index_17_patch = pointer;
            __asan_poison_memory_region((u8 *)data + available, 0x20000 - available);
            (void)patch(data, available);
            __asan_unpoison_memory_region(data, 0x20000);
        }
        detect(NULL, 0x20000); assert(!is_Nes);
#endif
    } else abort();
    free(data);
}
