/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "loader_layout.h"
#include "common.h"
#pragma GCC optimize ("Os")
bool loader_menu_size(uint32_t menu, uint32_t fonts, uint32_t cheats, uint32_t *size) {
  if (!size || !menu || !fonts || ((menu|fonts|cheats)&3) || menu>MAX_GBA_ROM_SIZE ||
      fonts>MAX_GBA_ROM_SIZE-menu || cheats>MAX_GBA_ROM_SIZE-menu-fonts) return false;
  *size=(menu+fonts+cheats+1023)&~1023u;
  return true;
}
bool loader_plan(uint32_t rom, uint32_t menu, uint32_t directsave,
                 uint32_t hole_start, uint32_t hole_size, t_loader_layout *layout) {
  if (!layout || !rom || rom>MAX_GBA_ROM_SIZE || menu>MAX_GBA_ROM_SIZE ||
      directsave>MAX_GBA_ROM_SIZE-menu || ((menu|directsave)&1023)) return false;
  unsigned required=menu+directsave;
  unsigned start=((rom+1023)&~1023u)+(rom<MAX_GBA_ROM_SIZE?1024:0);
  if (!required && start>MAX_GBA_ROM_SIZE) start=MAX_GBA_ROM_SIZE;
  unsigned space;
  if (start<=MAX_GBA_ROM_SIZE-required) {
    space=MAX_GBA_ROM_SIZE-start-directsave;
  } else {
    if ((hole_start&3) || hole_start>rom || hole_size>rom-hole_start || hole_size<required)
      return false;
    start=hole_start;
    space=hole_size-directsave;
  }
  space&=~1023u;
  if (menu>space) return false;
  *layout=(t_loader_layout){start,start+directsave,space,start,start+directsave+space};
  return true;
}
