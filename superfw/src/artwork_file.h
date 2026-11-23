/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef SUPERFW_ARTWORK_FILE_H
#define SUPERFW_ARTWORK_FILE_H
#include "artwork.h"
/* Candidate 0: mirrored, 1: CUSTOM. Root follows existing DSSMM /SYSTEM layout.
 * Absolute canonical ROM path required. Overlong/unsafe paths are rejected. */
bool artwork_path(char *out, size_t capacity, const char *rom, unsigned style, unsigned candidate);
/* Staging buffer must be word-aligned with even writable capacity. */
bool artwork_directory_path(char *out, size_t capacity, const char *directory, unsigned style);
bool artwork_load_directory(const char *directory, unsigned style, uint8_t *pixels, size_t capacity,
                            uint8_t *bmp_buffer, size_t bmp_capacity, artwork_info *info);
bool artwork_load(const char *rom, unsigned style, uint8_t *pixels, size_t capacity,
                  uint8_t *bmp_buffer, size_t bmp_capacity, artwork_info *info);
/* CUSTOM/filename lookup without opening the source ROM (NOR browsing). */
bool artwork_load_named(const char *rom, unsigned style, uint8_t *pixels, size_t capacity,
                        uint8_t *bmp_buffer, size_t bmp_capacity, artwork_info *info);
/* Title-only ROM-code fallback; four bytes, no terminating NUL required. */
bool artwork_load_gamecode(const char code[4], uint8_t *pixels, size_t capacity,
                          uint8_t *bmp_buffer, size_t bmp_capacity, artwork_info *info);
#endif
