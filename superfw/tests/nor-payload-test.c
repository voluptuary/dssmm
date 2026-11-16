#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#define MAX_GBA_ROM_SIZE (32u*1024*1024)
#define NOR_BLOCK_SIZE (4u*1024*1024)
#define DIRSAVE_REQ_SPACE 7168u
static const unsigned ingame_trampoline_payload_size=256;
typedef struct {unsigned hole_addr,hole_size,irqh_ops,save_mode;} t_patch;
typedef struct {unsigned romfs;} t_load_gba_info;
static t_patch patch;
static bool patch_found;
static const t_patch *get_game_patch(const t_load_gba_info *i) {(void)i;return patch_found?&patch:NULL;}
static bool supports_directsave(unsigned mode) {return mode!=0;}
#include "nor-payload.h"
int main(void) {
 t_load_gba_info info={MAX_GBA_ROM_SIZE};
 assert(!ingame_menu_avail_flash(&info)&&!dirsav_avail_flash(&info));
 patch_found=true;patch=(t_patch){0x1000,DIRSAVE_REQ_SPACE,1,1};
 assert(dirsav_avail_flash(&info)&&!ingame_menu_avail_flash(&info));
 patch.hole_size+=ingame_trampoline_payload_size;assert(ingame_menu_avail_flash(&info));
 patch.hole_addr++;assert(!dirsav_avail_flash(&info)&&!ingame_menu_avail_flash(&info));
 patch.hole_addr=MAX_GBA_ROM_SIZE-4;patch.hole_size=UINT32_MAX;assert(!dirsav_avail_flash(&info)&&!ingame_menu_avail_flash(&info));
 patch.hole_addr=MAX_GBA_ROM_SIZE+4;patch.hole_size=1;assert(!dirsav_avail_flash(&info)&&!ingame_menu_avail_flash(&info));
 info.romfs=28u*1024*1024;assert(dirsav_avail_flash(&info)&&ingame_menu_avail_flash(&info));
 puts("NOR payload availability: combined trampoline space, alignment and overflow bounds passed.");
}
