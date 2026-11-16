/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef DSSMM_UPDATE_UNLOCK_H
#define DSSMM_UPDATE_UNLOCK_H
#include <stdint.h>
#include <stdbool.h>
typedef struct {uint32_t began;uint8_t position;} update_unlock_state;
/* Feed fresh key-down events only; never auto-repeat or held-key chords. */
bool update_unlock_feed(update_unlock_state *state,unsigned keys,uint32_t frames);
#endif
