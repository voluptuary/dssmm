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
#endif
