/* SPDX-License-Identifier: GPL-3.0-or-later */
/* RIOT countdown: first decrement after one clock, then the selected divider;
 * after underflow the counter runs at CPU clock speed. No host wall clock. */
#include <stdint.h>
#include "types.h"
#include "vmachine.h"
#include "timer.h"
extern CLOCK clk;
static unsigned active, overflowed, irq;

void atari_timer_reset(void) {
    active = overflowed = irq = 0;
    timer_res = 10;
    timer_count = timer_clks = 0;
}

void set_timer(int shift, int count, int clkadj) {
    timer_res = shift;
    timer_count = (count & 255) << shift;
    timer_clks = (uint32_t)clk + (uint32_t)clkadj;
    active = 1;
    overflowed = irq = 0;
}

static BYTE timer_value(int clkadj) {
    uint32_t elapsed, divider, seed;
    if (!active) return 0;
    divider = 1u << timer_res;
    seed = (unsigned)timer_count >> timer_res;
    elapsed = (uint32_t)clk + (uint32_t)clkadj - (uint32_t)timer_clks;
    /* The legacy instruction-level bus can sample before a pending write. */
    if (elapsed & 0x80000000u) return seed;
    if (elapsed <= (unsigned)timer_count)
        return seed - ((elapsed + divider - 1) >> timer_res);
    if (!overflowed) {
        overflowed = 1;
        irq = 0x80;
    }
    return 255u - (elapsed - (unsigned)timer_count - 1u);
}

BYTE do_timer(int clkadj) {
    BYTE result = timer_value(clkadj);
    /* A read coinciding with underflow leaves the newly raised flag set. */
    if ((uint32_t)clk + (uint32_t)clkadj - (uint32_t)timer_clks !=
            (uint32_t)timer_count + 1u)
        irq = 0;
    return result;
}

BYTE do_timer_irq(int clkadj) {
    timer_value(clkadj);
    return irq;
}
