#ifndef BLUE_IMAGE_POLICY_H
#define BLUE_IMAGE_POLICY_H
#include <stdint.h>
/* Firmware-free stock-size packaging and whole-image sealing were prompted by
   FrankieT19's BLUE Test5 (08ab566). This DSSMM format/implementation is separate. */
#define BLUE_IMAGE_BYTES 0x2069DCu
#define BLUE_IMAGE_SEAL_OFFSET (BLUE_IMAGE_BYTES - 16u)
#define BLUE_IMAGE_SEAL_MAGIC "DSSMBL1"
static inline uint32_t BlueImageCrc(uint32_t crc, const unsigned char *data, unsigned bytes)
{
    while(bytes--) {
        crc ^= *data++;
        for(unsigned bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ ((crc & 1u) ? 0xEDB88320u : 0);
    }
    return crc;
}
#endif
