#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint32_t u32;
typedef int32_t s32;
typedef uint16_t u16;
typedef uint8_t u8;
static u32 width = 120;
#define LAUNCHER_THUMB_BMP_HEADER 54
#define LAUNCHER_THUMB_BMP_ALPHA_MARKER_OFFSET 6
#define LAUNCHER_THUMB_BMP_ALPHA_MARKER_0 'D'
#define LAUNCHER_THUMB_BMP_ALPHA_MARKER_1 'S'
#define LAUNCHER_THUMB_BMP_ALPHA_MARKER_2 'T'
#define LAUNCHER_THUMB_BMP_ALPHA_MARKER_3 '1'
#define LAUNCHER_THUMB_TRANSPARENT_SENTINEL 0x8000
static u32 Launcher_ThumbnailSourceWidth(void) { return width; }
static u32 Launcher_ThumbnailSourceHeight(void) { return 80; }
static u32 Launcher_ThumbnailReadSize(void) { return 54 + width * 80 * 2; }
#include "marked-artwork.impl.h"
static void le32(u8 *p, u32 value)
{
    for(unsigned i = 0; i < 4; i++) p[i] = (u8)(value >> (i * 8));
}
int main(void)
{
    static u16 storage[(54 + 120 * 80 * 2) / 2 + 1];
    u8 *bmp = (u8 *)storage;
    u16 *pixels = (u16 *)(bmp + 54);
    const unsigned invalid_bytes[] = {
        0, 1, 10, 11, 12, 13, 14, 15, 16, 17, 26, 27,
        18, 19, 20, 21, 22, 23, 24, 25, 28, 29, 30, 31, 32, 33
    };
    for(width = 80; width <= 120; width += 40) {
        memset(storage, 0, sizeof storage);
        bmp[0] = 'B'; bmp[1] = 'M';
        memcpy(bmp + 6, "DST1", 4);
        le32(bmp + 10, 54); le32(bmp + 14, 40);
        le32(bmp + 18, width); le32(bmp + 22, (u32)-80);
        bmp[26] = 1; bmp[28] = 16;
        pixels[0] = 0; pixels[1] = 0x7FFF; pixels[2] = 0x8001;
        storage[Launcher_ThumbnailReadSize() / 2] = 0xA55A;
        assert(Launcher_HasTransparentThumbnailMarker(bmp, Launcher_ThumbnailReadSize()));
        Launcher_TagTransparentThumbnailPixels(bmp, Launcher_ThumbnailReadSize());
        assert(pixels[0] == 0x8000 && pixels[1] == 0x7FFF && pixels[2] == 0x8001);
        pixels[0] = 0;
        for(unsigned i = 0; i < sizeof invalid_bytes / sizeof invalid_bytes[0]; i++) {
            unsigned offset = invalid_bytes[i];
            bmp[offset] ^= 1;
            assert(!Launcher_HasTransparentThumbnailMarker(bmp, Launcher_ThumbnailReadSize()));
            Launcher_TagTransparentThumbnailPixels(bmp, Launcher_ThumbnailReadSize());
            assert(pixels[0] == 0);
            bmp[offset] ^= 1;
        }
        for(unsigned i = 6; i < 10; i++) {
            bmp[i] ^= 1;
            Launcher_TagTransparentThumbnailPixels(bmp, Launcher_ThumbnailReadSize());
            assert(pixels[0] == 0);
            bmp[i] ^= 1;
        }
        le32(bmp + 22, 80);
        assert(!Launcher_HasTransparentThumbnailMarker(bmp, Launcher_ThumbnailReadSize()));
        le32(bmp + 22, (u32)-80);
        assert(!Launcher_HasTransparentThumbnailMarker(bmp, Launcher_ThumbnailReadSize() - 1));
        assert(storage[Launcher_ThumbnailReadSize() / 2] == 0xA55A);
    }
    assert(!Launcher_HasTransparentThumbnailMarker(NULL, 54));
    puts("Marked artwork: complete headers, malformed fields, legacy preservation, sentinels and canaries passed.");
}
