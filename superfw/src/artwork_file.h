/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef SUPERFW_ARTWORK_FILE_H
#define SUPERFW_ARTWORK_FILE_H
#include "artwork.h"
/* Candidate 0: mirrored, 1: CUSTOM. Root follows existing DSSMM /SYSTEM layout.
 * Absolute canonical ROM path required. Overlong/unsafe paths are rejected. */
bool artwork_path(char *out, size_t capacity, const char *rom, unsigned style, unsigned candidate);
bool artwork_load(const char *rom, unsigned style, uint8_t *pixels, size_t capacity, artwork_info *info);
#endif
