/* DSSMM artwork reader. SPDX-License-Identifier: GPL-3.0-or-later
 * Input format: voluptuary/dsscc, documented in docs/DSSCC-FORMAT-AUDIT.md.
 * Original decoder implementation; no generator source is incorporated. */
#include "artwork.h"
#include <string.h>
#ifdef __GBA__
#include "gbahw.h"
#endif
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
  uint8_t header[54], rows[480];
  if (!read || !pixels || ((uintptr_t)pixels & 1) || !info || file_size < sizeof(header) ||
      !read(context, 0, header, sizeof(header))) return false;
  uint32_t width = le32(header + 18), offset = le32(header + 10);
  /* Fixed dimensions bound every size/offset calculation and scratch buffer. */
  uint32_t height = le32(header + 22);
  bool bottom_up = height == 80;
  if (memcmp(header, "BM", 2) || le32(header + 14) != 40 ||
      (width != 80 && width != 120) || (height != 80 && height != (uint32_t)-80) ||
      le16(header + 26) != 1 || le16(header + 28) != 16 ||
      le32(header + 30) != 0 || offset < sizeof(header) ||
      offset > file_size || width * 160 > file_size - offset ||
      (le32(header + 2) && le32(header + 2) != file_size) ||
      (le32(header + 34) && le32(header + 34) != width * 160) ||
      capacity < width * 80) return false;
  bool transparent = memcmp(header + 6, "DST1", 4) == 0;
  /* Decode in bounded two-row chunks from the contiguous file buffer. */
  for (unsigned y = 0; y < 80; y += 2) {
    if (!read(context, offset + y * width * 2, rows, width * 4)) return false;
    for (unsigned ry = 0; ry < 2; ++ry) {
      unsigned source_y = y + ry;
      unsigned target_y = bottom_up ? 79 - source_y : source_y;
      const uint8_t *row = rows + ry * width * 2;
      for (unsigned x = 0; x < width; x += 2) {
        uint16_t a = le16(row + x * 2), b = le16(row + x * 2 + 2);
        /* BI_RGB 16-bit BMP uses RGB555; bit 15 is unused. */
        a &= 0x7fff;
        b &= 0x7fff;
        unsigned lo = transparent && !a ? 0 : pixel_index(a);
        unsigned hi = transparent && !b ? 0 : pixel_index(b);
        /* Cartridge SDRAM and GBA VRAM require halfword writes. */
        ((volatile uint16_t *)pixels)[(target_y * width + x) / 2] = lo | (hi << 8);
      }
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
#ifdef __GBA__
    if (!info->transparent) {
      dma_memcpy16(dst, src, info->width / 2);
      continue;
    }
#endif
    for (unsigned col = 0; col < info->width / 2; ++col) {
      uint16_t pair = src[col], bg = dst[col];
      if (!(pair & 255)) pair |= bg & 255;
      if (!(pair & 0xff00)) pair |= bg & 0xff00;
      dst[col] = pair;
    }
  }
  return true;
}

bool artwork_blit_scaled(volatile uint16_t *frame, const uint16_t *pixels,
                         const artwork_info *info, unsigned x, unsigned y,
                         unsigned width, unsigned height) {
  if (!frame || !pixels || !info || ((uintptr_t)frame & 1) || ((uintptr_t)pixels & 1) ||
      (x & 1) || !width || (width & 1) || !height || width > 240 || height > 160 ||
      (info->width != 80 && info->width != 120) || info->height != 80 ||
      x > 240U - width || y > 160U - height) return false;
  /* GBA has no hardware divide. Reuse horizontal samples across all rows. */
  uint8_t source_x[240];
  for (unsigned col = 0; col < width; ++col)
    source_x[col] = col * info->width / width;
  for (unsigned row = 0; row < height; ++row) {
    unsigned sy = row * info->height / height;
    volatile uint16_t *dst = frame + (y + row) * 120 + x / 2;
    for (unsigned col = 0; col < width; col += 2) {
      unsigned sx0 = source_x[col];
      unsigned sx1 = source_x[col + 1];
      uint8_t p0 = ((const uint8_t *)pixels)[sy * info->width + sx0];
      uint8_t p1 = ((const uint8_t *)pixels)[sy * info->width + sx1];
      uint16_t pair = p0 | ((uint16_t)p1 << 8);
      if (!p0) pair = (pair & 0xff00) | (dst[col / 2] & 0x00ff);
      if (!p1) pair = (pair & 0x00ff) | (dst[col / 2] & 0xff00);
      dst[col / 2] = pair;
    }
  }
  return true;
}
