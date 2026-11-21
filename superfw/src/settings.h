/*
 * Copyright (C) 2024 David Guillen Fandos <david@davidgf.net>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdint.h>
#include <stdbool.h>

#include "common.h"
#include "fonts/font_render.h"
#include "launcher_profile.h"

typedef struct {
  const char * const cname;
  uint16_t mask;
} t_combo_key;

typedef struct {
  uint16_t fg_color;     // Foreground elements color
  uint16_t bg_color;     // Background color
  uint16_t ft_color;     // Font color
  uint16_t hi_color;     // Item/Buttom highlight
  uint16_t hi_blend;     // Menu highlight color (browser)
  uint16_t sh_color;     // Menu shadow/disabled color
} t_theme_colors;

/* Keep upstream numeric IDs stable for existing ui-settings.txt files. */
enum {
  THEME_DSSMM_LIGHT = 6,
  THEME_DSSMM_DARK,
  THEME_COUNT
};
extern const t_theme_colors themes[THEME_COUNT];
#define ACCENT_COUNT 5
extern const char * const accent_names[ACCENT_COUNT];
extern uint8_t menu_accent;
extern uint8_t carousel_wrap;
extern char menu_username[LAUNCHER_NAME_BYTES];
extern uint32_t menu_clock_seed;
extern uint8_t menu_font; /* Native or optional DSSMM printable ASCII font. */
extern uint8_t clean_list,hide_system_files,menu_clock_24,menu_topbar;
extern uint8_t artwork_mode; /* 0=list, 1=title, 2=box; Start in SD browser */
extern uint8_t browser_view;  /* 0=list, 1=horizontal carousel, 2=vertical carousel */
t_theme_colors menu_theme_colors(unsigned theme, unsigned accent);

extern const t_combo_key hotkey_list[13];
#define hotkey_listcnt (sizeof(hotkey_list)/sizeof(hotkey_list[0]))

extern const uint8_t animspd_lut[5];
#define animspd_cnt (sizeof(animspd_lut)/sizeof(animspd_lut[0]))



// ROM loading and launching config settings
typedef struct {
  unsigned patch_policy;     // Can only be PatchDatabase, PatchEngine or PatchNone
  bool use_igm;
  bool use_rtc;
  bool use_dsaving;
} t_rom_load_settings;

typedef struct {
  bool use_cheats;
  uint32_t rtcts;
} t_rom_launch_settings;

// Menu settings
extern uint8_t menu_theme;
extern uint8_t lang_id;
extern uint8_t recent_menu;
extern uint8_t hide_hidden;
extern uint8_t anim_speed;

// Defaults/Settings
extern t_patch_policy patcher_default;
extern uint8_t boot_bios_splash;
extern uint8_t use_slowld;
extern uint8_t use_fastew;
extern uint8_t use_verify_nor;
extern uint8_t backup_sram_default;
extern uint8_t hotkey_combo;
extern uint8_t enable_cheats;
extern uint8_t autoload_default;
extern uint8_t autosave_default;
extern uint8_t autosave_prefer_ds;
extern uint8_t ingamemenu_default;
extern uint8_t rtcpatch_default;
extern uint8_t rtcspeed_default;
extern uint32_t rtcvalue_default;

// Setting load/store
bool save_settings(void);
void settings_save_request(void);
bool settings_save_pending(void);
bool settings_save_preparing(void);
bool settings_save_poll(void);
bool settings_save_flush(void);
void load_settings();
bool settings_loaded_ok(void);

// ROM-specific setting load/store
bool load_rom_settings(const char *fn, t_rom_load_settings *rld, t_rom_launch_settings *rlh);
bool save_rom_settings(const char *fn, const t_rom_load_settings *rld, const t_rom_launch_settings *rlh);

#endif

