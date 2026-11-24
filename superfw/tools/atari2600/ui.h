/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef DSSMM_ATARI_UI_H
#define DSSMM_ATARI_UI_H
int atari_menu(void); /* 0 resumes; 1 requests the core's normal restart. */
int atari_return_ready(void) __attribute__((noinline));
void atari_exit(void) __attribute__((noreturn));
void atari_bad_rom(void) __attribute__((noreturn));
#endif
