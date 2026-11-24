/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stddef.h>
#include "menu_workspace.h"
#include "artwork.h"
#include "fatfs/ff.h"

/* Byte-writable GBA EWRAM. Settings and the optional selected-image hot copy
 * never need simultaneous storage; SDRAM retains the authoritative artwork. */
static union {
    char settings[MENU_SETTINGS_CAPACITY];
    uint16_t artwork[ARTWORK_MAX_PIXELS / 2];
} menu_workspace __attribute__((section(".sbss"), aligned(__alignof__(FIL))));
static bool settings_busy;
static unsigned workspace_generation;

char *menu_workspace_begin_settings(void) {
    if (settings_busy) return NULL;
    settings_busy = true;
    ++workspace_generation;
    return menu_workspace.settings;
}

void menu_workspace_end_settings(void) {
    settings_busy = false;
}

uint16_t *menu_workspace_artwork(unsigned *generation) {
    *generation = workspace_generation;
    return settings_busy ? NULL : menu_workspace.artwork;
}
