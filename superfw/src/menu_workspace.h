/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef SUPERFW_MENU_WORKSPACE_H
#define SUPERFW_MENU_WORKSPACE_H
#include <stdint.h>
#define MENU_SETTINGS_CAPACITY 8192
/* A settings lease excludes artwork until parsing or the entire write finishes.
 * The generation changes whenever a settings lease replaces cached pixels. */
char *menu_workspace_begin_settings(void);
void menu_workspace_end_settings(void);
uint16_t *menu_workspace_artwork(unsigned *generation);
#endif
