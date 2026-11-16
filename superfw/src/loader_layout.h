/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef SUPERFW_LOADER_LAYOUT_H
#define SUPERFW_LOADER_LAYOUT_H
#include <stdbool.h>
#include <stdint.h>
typedef struct {
  uint32_t directsave, menu, menu_space, gap_start, gap_end;
} t_loader_layout;
/* Rounded menu/font/cheat allocation; rejects overflow and unaligned data. */
bool loader_menu_size(uint32_t menu, uint32_t fonts, uint32_t cheats, uint32_t *size);
/* Cartridge offsets only. A supplied hole must lie inside the ROM. */
bool loader_plan(uint32_t rom, uint32_t menu, uint32_t directsave,
                 uint32_t hole_start, uint32_t hole_size, t_loader_layout *layout);
#endif
