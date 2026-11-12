#ifndef KERNEL_BOOT_POLICY_H
#define KERNEL_BOOT_POLICY_H
#include <stdint.h>

/* Keep the Pogoshell argument block above this boundary intact. */
#define KERNEL_EWRAM_PAYLOAD_LIMIT 0x3FBF8UL
#define KERNEL_PLUGIN_ROM_LIMIT 0x02000000UL

/* Do not assume native hardware mapping applies to generic plugins. */
static inline int KernelPluginRangeFits(uint32_t address, uint32_t bytes)
{
    return address <= KERNEL_PLUGIN_ROM_LIMIT && bytes <= KERNEL_PLUGIN_ROM_LIMIT - address;
}

static inline int KernelPluginPayloadValid(const unsigned char *data, uint32_t bytes, uint32_t mode)
{
    if(mode != 2 && mode != 4) return 1;
    if(!data || !bytes) return 0;
    if(mode == 2) return bytes <= KERNEL_EWRAM_PAYLOAD_LIMIT;
    if(bytes < 4 || data[0] != 0x10) return 0;
    uint32_t output = data[1] | (uint32_t)data[2] << 8 | (uint32_t)data[3] << 16;
    uint32_t input = 4, produced = 0;
    if(!output || output > KERNEL_EWRAM_PAYLOAD_LIMIT) return 0;
    while(produced < output) {
        if(input >= bytes) return 0;
        unsigned flags = data[input++];
        for(unsigned bit = 0; bit < 8 && produced < output; bit++) {
            if(flags & (0x80u >> bit)) {
                if(bytes - input < 2) return 0;
                unsigned first = data[input++], second = data[input++];
                uint32_t length = (first >> 4) + 3;
                uint32_t distance = ((first & 15) << 8 | second) + 1;
                if(distance > produced || length > output - produced) return 0;
                produced += length;
            } else {
                if(input >= bytes) return 0;
                input++; produced++;
            }
        }
    }
    return 1;
}
#endif
