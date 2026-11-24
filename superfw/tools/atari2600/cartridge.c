/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Experimental 2/4 KiB and standard F8/F6 input through the kernel's Pogoshell ABI. */
#include <stdint.h>
#include <string.h>
#include "types.h"
#include "vmachine.h"
#include "resource.h"

int loadCart(char *name) {
    const volatile uint32_t *abi = (const volatile uint32_t *)0x0203fbf8;
    uint32_t size = abi[0], address = abi[1];
    (void)name;
    if (abi[2] != 0xfab0babe || abi[3] != 2 ||
        (size != 2048 && size != 4096 && size != 8192 && size != 16384) || (address & 3) ||
        address < 0x08000000 || address > 0x08e00000 - size)
        return -1;
    app_data.bank = size == 16384 ? 1 : size == 8192 ? 2 : 0;
    memcpy(cart, (const void *)address, size);
    memcpy(theRom, cart + (size > 4096 ? size - 4096 : 0),
           size > 4096 ? 4096 : size);
    if (size == 2048)
        memcpy(theRom + 2048, cart, 2048);
    return 0;
}
