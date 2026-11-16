/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "common.h"
#include "loader_layout.h"
int main(void) {
 uint32_t bytes;assert(loader_menu_size(216452,1047800,64,&bytes));assert(bytes==((216452+1047800+64+1023)&~1023));
 assert(loader_menu_size(4,1020,0,&bytes)&&bytes==1024);
 assert(loader_menu_size(4,1020,4,&bytes)&&bytes==2048);
 assert(!loader_menu_size(4,1020,1,&bytes));assert(!loader_menu_size(UINT32_MAX,4,4,&bytes));
 assert(!loader_menu_size(4,MAX_GBA_ROM_SIZE,4,&bytes));assert(!loader_menu_size(4,4,UINT32_MAX,&bytes));
 t_loader_layout plan;const unsigned menu=1024*1024;
 unsigned rom=MAX_GBA_ROM_SIZE-menu-1024;
 assert(loader_plan(rom,menu,0,0,0,&plan));assert(plan.menu==MAX_GBA_ROM_SIZE-menu);
 assert(!loader_plan(rom,menu+1024,0,0,0,&plan));
 assert(loader_plan(rom,menu+1024,0,2000,menu+2048,&plan));assert(plan.menu==2000&&plan.menu_space>=menu+1024);
 assert(!loader_plan(rom,menu+1024,0,2001,menu+2048,&plan));
 assert(!loader_plan(rom,menu+1024,0,UINT32_MAX,UINT32_MAX,&plan));
 assert(loader_plan(MAX_GBA_ROM_SIZE-1,0,0,0,0,&plan)&&plan.gap_start==MAX_GBA_ROM_SIZE);
 assert(loader_plan(MAX_GBA_ROM_SIZE,0,0,0,0,&plan)&&plan.gap_end==MAX_GBA_ROM_SIZE);
 assert(!loader_plan(0,menu,0,0,0,&plan));assert(!loader_plan(MAX_GBA_ROM_SIZE+1,0,0,0,0,&plan));
 assert(loader_plan(10003,menu,7*1024,0,0,&plan));assert(plan.menu==plan.directsave+7*1024);
 assert(plan.menu_space>=menu&&plan.gap_end<=MAX_GBA_ROM_SIZE);
 t_loader_layout before=plan;assert(!loader_plan(rom,MAX_GBA_ROM_SIZE,7*1024,0,0,&plan));assert(!memcmp(&before,&plan,sizeof(plan)));
 return 0;
}
