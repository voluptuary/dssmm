/* DSSMM artwork reader. SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef SUPERFW_ARTWORK_H
#define SUPERFW_ARTWORK_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/* Background palette 20..235 avoids native UI (0..19) and in-game (240..244).
 * Index zero in decoded data is a transparent sentinel, never opaque black. */
#define ARTWORK_PALETTE_BASE 20
#define ARTWORK_PALETTE_SIZE 216
#define ARTWORK_MAX_PIXELS (120 * 80)
typedef bool (*artwork_read_fn)(void *context, uint32_t offset, void *data, size_t size);
typedef struct {
  uint16_t width, height;
  bool transparent;
} artwork_info;
/* Exact random-access reads; caller owns halfword-aligned storage. Output is invalid on failure.
 * Accept only DSSCC top-down, uncompressed 16-bit 120x80 or 80x80 BMPs. */
bool artwork_decode(artwork_read_fn read, void *context, uint32_t file_size,
                    uint8_t *pixels, size_t capacity, artwork_info *info);
uint16_t artwork_palette_color(unsigned index);
/* Composite into a 240x160 indexed frame with halfword-only stores. */
bool artwork_blit_frame(volatile uint16_t *frame, const uint16_t *pixels,
                       const artwork_info *info, unsigned x, unsigned y);
#endif
