#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t u32;typedef uint16_t u16;typedef uint8_t u8;typedef volatile uint16_t vu16;
#define BANK 0x800000u
#define PSRAM_CAPACITY_BYTES (BANK * PATCH_BANKS)
static u8 physical[PSRAM_CAPACITY_BYTES];
static u16 aperture[(BANK + 16) / 2];
#define PSRAMBase_S98 ((u8 *)aperture)
static u32 selected, is_NORpatch, windows_offset;
static u8 cache_store[0x20000 + 16];
#define pReadCache cache_store
static void SetPSRampage(u16 page)
{
    assert(!(page % 0x1000) && page / 0x1000 < PATCH_BANKS);
    memcpy(physical + selected * BANK, aperture, BANK);
    selected = page / 0x1000;
    memcpy(aperture, physical + selected * BANK, BANK);
}
#include "patch-write.impl.h"
int main(int argc, char **argv)
{
    assert(argc == 2);
    u16 words[] = {0x1234,0x5678,0x9ABC};
    memset(physical, 0xA7, sizeof physical);memset(aperture, 0xA7, sizeof aperture);
    if(!strcmp(argv[1], "banks")) {
        for(unsigned bank = 1; bank < PATCH_BANKS; bank++) {
            Write(bank * BANK - 2, (const u8 *)words, sizeof words);
            assert(!memcmp(physical + bank * BANK - 2, words, sizeof words));
            assert(!selected && aperture[BANK / 2] == 0xA7A7);
        }
    } else if(!strcmp(argv[1], "last")) {
        Write(PSRAM_CAPACITY_BYTES - 2, (const u8 *)words, 2);
        assert(!selected && !memcmp(physical + PSRAM_CAPACITY_BYTES - 2, words, 2));
        assert(aperture[BANK / 2] == 0xA7A7);
    } else if(!strcmp(argv[1], "range")) {
        Write(PSRAM_CAPACITY_BYTES, (const u8 *)words, 2);assert(!selected);
    } else if(!strcmp(argv[1], "span")) {
        Write(PSRAM_CAPACITY_BYTES - 2, (const u8 *)words, sizeof words);
        assert(!selected && physical[PSRAM_CAPACITY_BYTES - 2] == 0xA7);
    } else if(!strcmp(argv[1], "huge")) {
        Write(PSRAM_CAPACITY_BYTES - 2, (const u8 *)words, UINT32_MAX);
        assert(!selected && physical[PSRAM_CAPACITY_BYTES - 2] == 0xA7);
    } else if(!strcmp(argv[1], "null")) {
        Write(0, NULL, 2);assert(!selected && aperture[0] == 0xA7A7);
    } else if(!strcmp(argv[1], "nor")) {
        memset(pReadCache, 0xCC, sizeof cache_store);is_NORpatch = 1;
        Write(0x1FFFE, (const u8 *)words, sizeof words);
        assert(!memcmp(pReadCache + 0x1FFFE, words, 2));
        for(unsigned i = 0x20000; i < sizeof cache_store; i++) assert(pReadCache[i] == 0xCC);
        memset(pReadCache, 0xCC, sizeof cache_store);windows_offset = 0x20000;
        Write(0x1FFFE, (const u8 *)words, sizeof words);
        assert(!memcmp(pReadCache, words + 1, 4) && pReadCache[4] == 0xCC);
        const int offsets[] = {-8,0,2,0x1FFF8,0x1FFFE,0x20000,0x20002};
        const unsigned lengths[] = {0,2,4,6};
        for(unsigned a = 0; a < sizeof offsets / sizeof offsets[0]; a++) {
            for(unsigned n = 0; n < sizeof lengths / sizeof lengths[0]; n++) {
                memset(pReadCache, 0xCC, sizeof cache_store);
                u32 address = windows_offset + offsets[a];Write(address, (const u8 *)words, lengths[n]);
                for(unsigned word = 0; word < 0x10000; word++) {
                    u32 absolute = windows_offset + word * 2;u16 expected = 0xCCCC;
                    if(absolute >= address && absolute - address < lengths[n]) expected = words[(absolute - address) / 2];
                    assert(((u16 *)pReadCache)[word] == expected);
                }
                for(unsigned i = 0x20000; i < sizeof cache_store; i++) assert(pReadCache[i] == 0xCC);
            }
        }
    } else abort();
}
