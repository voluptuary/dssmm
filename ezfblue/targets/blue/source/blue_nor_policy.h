#ifndef BLUE_NOR_POLICY_H
#define BLUE_NOR_POLICY_H
#include <stdint.h>

#define BLUE_NOR_FIRST 0x00800000UL
#define BLUE_NOR_END 0x08000000UL
#define BLUE_NOR_BLOCK 0x00020000UL
#define BLUE_NOR_MAP_SIZE 960U

static inline __attribute__((always_inline)) int BlueNorValidSpan(uint32_t page, uint32_t bytes)
{
    if(page < BLUE_NOR_FIRST / BLUE_NOR_BLOCK || page >= BLUE_NOR_END / BLUE_NOR_BLOCK)
        return 0;
    uint32_t start = page * BLUE_NOR_BLOCK;
    return bytes && !(bytes % 0x40000) && bytes <= BLUE_NOR_END - start;
}

static inline uint32_t BlueNorFindFree(const unsigned char *map, uint32_t bytes)
{
    if(!map || !bytes || bytes % 0x40000 || bytes > BLUE_NOR_END - BLUE_NOR_FIRST) return 0;
    uint32_t needed = bytes / BLUE_NOR_BLOCK;
    uint32_t run = 0;
    for(uint32_t page = 0; page < BLUE_NOR_MAP_SIZE; page++) {
        run = map[page] ? 0 : run + 1;
        if(run == needed) return BLUE_NOR_FIRST + (page + 1 - needed) * BLUE_NOR_BLOCK;
    }
    return 0;
}

static inline int BlueNorMark(unsigned char *map, uint32_t page, uint32_t bytes)
{
    if(!map || !BlueNorValidSpan(page, bytes)) return 0;
    uint32_t first = page - BLUE_NOR_FIRST / BLUE_NOR_BLOCK;
    uint32_t count = bytes / BLUE_NOR_BLOCK;
    for(uint32_t index = first; index < first + count; index++)
        if(map[index]) return 0;
    for(uint32_t index = first; index < first + count; index++) map[index] = 0xFF;
    return 1;
}
/* Reserve the complete add-on extent, including a patch just before a block end.
   Inspired by FrankieT19 BLUE Test3's write-plan review; no upstream code copied. */
static inline int BlueNorAllocation(uint32_t size, int patched, uint32_t trim, uint32_t *bytes)
{
    if(!bytes || size < 0xC0 || size > 0x4000000) return 0;
    uint32_t end = size;
    if(patched) {
        if(trim < 0xC0 || trim % 4 || trim > 0x2000000 - 0x2000 ||
           trim % BLUE_NOR_BLOCK > BLUE_NOR_BLOCK - 0x2000) return 0;
        if(end < trim + 0x2000) end = trim + 0x2000;
    }
    *bytes = (end + 0x3FFFFu) & ~0x3FFFFu;
    return 1;
}
#endif
