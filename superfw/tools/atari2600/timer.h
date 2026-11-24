/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef DSSMM_ATARI_TIMER_H
#define DSSMM_ATARI_TIMER_H
void atari_timer_reset(void);
unsigned char do_timer_irq(int clkadj);
#endif
