/* DSSMM artwork reader. SPDX-License-Identifier: GPL-3.0-or-later
 * Input format: voluptuary/dsscc, documented in docs/DSSCC-FORMAT-AUDIT.md.
 * Original decoder implementation; no generator source is incorporated. */
#include "artwork.h"
#include <string.h>
static uint16_t le16(const uint8_t *p) { return p[0] | ((uint16_t)p[1] << 8); }
static uint32_t le32(const uint8_t *p) {
  return le16(p) | ((uint32_t)le16(p + 2) << 16);
}
uint16_t artwork_palette_color(unsigned index) {
  if (index >= ARTWORK_PALETTE_SIZE) return 0;
  unsigned r = index % 6, g = (index / 6) % 6, b = index / 36;
  return ((r * 31 + 2) / 5) | (((g * 31 + 2) / 5) << 5) |
         (((b * 31 + 2) / 5) << 10);
}
static uint8_t pixel_index(uint16_t pixel) {
  unsigned r = ((pixel & 31) * 5 + 15) / 31;
  unsigned g = (((pixel >> 5) & 31) * 5 + 15) / 31;
  unsigned b = (((pixel >> 10) & 31) * 5 + 15) / 31;
  return ARTWORK_PALETTE_BASE + r + 6 * g + 36 * b;
}
bool artwork_decode(artwork_read_fn read, void *context, uint32_t file_size,
                    uint8_t *pixels, size_t capacity, artwork_info *info) {
  uint8_t header[54], row[240];
  if (!read || !pixels || ((uintptr_t)pixels & 1) || !info || file_size < sizeof(header) ||
      !read(context, 0, header, sizeof(header))) return false;
  uint32_t width = le32(header + 18), offset = le32(header + 10);
  /* Fixed dimensions bound every size/offset calculation and scratch buffer. */
  if (memcmp(header, "BM", 2) || le32(header + 14) != 40 ||
      (width != 80 && width != 120) || le32(header + 22) != (uint32_t)-80 ||
      le16(header + 26) != 1 || le16(header + 28) != 16 ||
      le32(header + 30) != 0 || offset < sizeof(header) ||
      offset > file_size || width * 160 > file_size - offset ||
      le32(header + 2) != file_size || le32(header + 34) != width * 160 ||
      capacity < width * 80) return false;
  bool transparent = memcmp(header + 6, "DST1", 4) == 0;
  for (unsigned y = 0; y < 80; ++y) {
    if (!read(context, offset + y * width * 2, row, width * 2)) return false;
    for (unsigned x = 0; x < width; x += 2) {
      uint16_t a = le16(row + x * 2), b = le16(row + x * 2 + 2);
      if ((a | b) & 0x8000) return false;
      unsigned lo = transparent && !a ? 0 : pixel_index(a);
      unsigned hi = transparent && !b ? 0 : pixel_index(b);
      /* Cartridge SDRAM and GBA VRAM require halfword writes. */
      ((volatile uint16_t *)pixels)[(y * width + x) / 2] = lo | (hi << 8);
    }
  }
  *info = (artwork_info){ width, 80, transparent };
  return true;
}

bool artwork_blit_frame(volatile uint16_t *frame, const uint16_t *pixels,
                       const artwork_info *info, unsigned x, unsigned y) {
  if (!frame || !pixels || !info || ((uintptr_t)frame & 1) || ((uintptr_t)pixels & 1) ||
      (x & 1) || (info->width != 80 && info->width != 120) || info->height != 80 ||
      x > 240U - info->width || y > 160U - info->height) return false;
  for (unsigned row = 0; row < info->height; ++row) {
    volatile uint16_t *dst = frame + (y + row) * 120 + x / 2;
    const uint16_t *src = pixels + row * info->width / 2;
    for (unsigned col = 0; col < info->width / 2; ++col) {
      uint16_t pair = src[col], bg = dst[col];
      if (!(pair & 255)) pair |= bg & 255;
      if (!(pair & 0xff00)) pair |= bg & 0xff00;
      dst[col] = pair;
    }
  }
  return true;
}
