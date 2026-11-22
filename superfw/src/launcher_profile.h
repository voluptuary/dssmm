/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef DSSMM_LAUNCHER_PROFILE_H
#define DSSMM_LAUNCHER_PROFILE_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define LAUNCHER_NAME_BYTES 49
/* First line, at most12 complete Unicode characters; optional UTF8 BOM. */
void launcher_name_parse(char out[LAUNCHER_NAME_BYTES],const char *data,size_t bytes);
/* Last-played caption baseline matches native DE one/two/three-line blocks. */
unsigned launcher_title_top(unsigned rows,unsigned dssmm_font);
/* One visible save interval; new writes add only a few frames. */
typedef struct { uint32_t until; bool visible,preparing; } launcher_notice;
void launcher_notice_update(launcher_notice *notice,uint32_t frame,bool preparing);
/* Software menu clock: no powered-off/battery clock is provided by this card. */
void launcher_clock_set(uint32_t seconds);
uint32_t launcher_clock_get(void);
#endif
