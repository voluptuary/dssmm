/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef DSSMM_BACKGROUND_H
#define DSSMM_BACKGROUND_H
#include <stdint.h>
#include <stdbool.h>
/* Palette0..15 reserved for background; logo screen supplies its own palette. */
void dssmm_background_palette(volatile uint16_t *palette,bool dark);
enum {DSSMM_BG_LIST,DSSMM_BG_START,DSSMM_BG_SETTINGS,DSSMM_BG_HORIZONTAL,
      DSSMM_BG_VERTICAL,DSSMM_BG_GRID,DSSMM_BG_HELP};
void dssmm_background_draw(volatile uint8_t *frame,bool dark,unsigned kind);
void dssmm_background_clear(volatile uint8_t *frame,bool dark,unsigned x,unsigned y,unsigned width,unsigned height);
#endif
