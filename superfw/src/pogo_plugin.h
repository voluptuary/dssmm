/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef DSSMM_POGO_PLUGIN_H
#define DSSMM_POGO_PLUGIN_H
#include <stdbool.h>
#include <stdint.h>
/* Portable Pogoshell ABI, independently implemented from the format used by
 * DSSMM/DS Style on EZ-FLASH. No EZ-FLASH storage protocol is used. */
/* Recognized FluBBa ARM startup: retain safe SuperCard ROM wait states. */
bool pogo_fix_waitcnt(uint32_t *data,uint32_t size);
bool pogo_layout(uint32_t plugin_size, uint32_t file_size,
                 uint32_t *file_offset, uint32_t *table_offset);
bool pogo_ram_valid(const uint8_t *data, uint32_t size, unsigned mode);
void pogo_arguments(uint32_t ram[24], uint32_t table[22], const char *plugin,
                    const char *filename, uint32_t file_offset,
                    uint32_t file_size, uint32_t table_offset);
#endif
