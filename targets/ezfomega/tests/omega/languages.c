#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#include "launcher_runtime_text.h"
int main(void)
{
    unsigned count = sizeof launcher_language_packs / sizeof launcher_language_packs[0];
    for(unsigned language = 0; language < count; language++) {
        for(unsigned item = 0; item < LTXT_TOTAL; item++)
            assert(launcher_language_packs[language].text[item] != NULL);
    }
    printf("Omega language packs: %u packs, %u entries each; no shifted/missing tail entries.\n", count, LTXT_TOTAL);
}
