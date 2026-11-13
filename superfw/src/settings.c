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

#include <string.h>
#include <stdlib.h>

#include "settings.h"
#include "fatfs/ff.h"
#include "common.h"
#include "nanoprintf.h"
#include "util.h"

#pragma GCC optimize ("Os")

unsigned lang_lookup(uint16_t code);
uint16_t lang_getcode();

const t_theme_colors themes[THEME_COUNT] = {
  { RGB2GBA(0xaaaaaa), RGB2GBA(0xffffff), RGB2GBA(0x000000), RGB2GBA(0xcccccc), RGB2GBA(0x9999bb), RGB2GBA(0xc08888) }, // White
  { RGB2GBA(0xeca551), RGB2GBA(0xe7c092), RGB2GBA(0x000000), RGB2GBA(0xbda27b), RGB2GBA(0x90816e), RGB2GBA(0x615d58) }, // Orange
  { RGB2GBA(0x26879c), RGB2GBA(0x8fb1b8), RGB2GBA(0x000000), RGB2GBA(0x5296a5), RGB2GBA(0x1d7f95), RGB2GBA(0x6f8185) }, // Blue
  { RGB2GBA(0x308855), RGB2GBA(0x88aa99), RGB2GBA(0x000000), RGB2GBA(0x778888), RGB2GBA(0x777777), RGB2GBA(0x606060) }, // Green
  { RGB2GBA(0xad11c8), RGB2GBA(0xe47af6), RGB2GBA(0x000000), RGB2GBA(0xad5dc6), RGB2GBA(0x724095), RGB2GBA(0x72667a) }, // Purple
  { RGB2GBA(0x222222), RGB2GBA(0x444444), RGB2GBA(0xeeeeee), RGB2GBA(0x737573), RGB2GBA(0xaaaaaa), RGB2GBA(0x606060) }, // Dark
  /* DSSMM presentation starts here; native palette roles remain unchanged.
   * These original palettes require no borrowed graphics or extra RAM buffers.
   * Accents stay readable with the renderer's existing black/white UI masks. */
  [THEME_DSSMM_LIGHT] = { RGB2GBA(0xd8e0e8), RGB2GBA(0xf8f8f8), RGB2GBA(0x182028), RGB2GBA(0x80b8e8), RGB2GBA(0x90b8d8), RGB2GBA(0x788088) },
  [THEME_DSSMM_DARK] = { RGB2GBA(0x303840), RGB2GBA(0x182028), RGB2GBA(0xf0f0f0), RGB2GBA(0x5088b8), RGB2GBA(0x406080), RGB2GBA(0x889098) },
};

/* Default preserves every upstream palette. Accent changes only selection roles. */
const char * const accent_names[ACCENT_COUNT] = {
  "Default", "Blue", "Green", "Orange", "Purple"
};
static const uint16_t accent_colors[ACCENT_COUNT - 1][2] = {
  { RGB2GBA(0x80b8e8), RGB2GBA(0x6098c8) },
  { RGB2GBA(0x88c898), RGB2GBA(0x589868) },
  { RGB2GBA(0xe8b878), RGB2GBA(0xb88848) },
  { RGB2GBA(0xc0a0e0), RGB2GBA(0x9878b8) },
};
uint8_t menu_accent = 0;
uint8_t artwork_mode = 1;
#define UI_SETTINGS_VERSION 3
#define UI_SETTINGS_DSSMM_DEFAULTS_VERSION 2
static uint8_t ui_settings_version = 0;

t_theme_colors menu_theme_colors(unsigned theme, unsigned accent) {
  t_theme_colors result = themes[theme < THEME_COUNT ? theme : 0];
  if (accent > 0 && accent < ACCENT_COUNT) {
    result.hi_color = accent_colors[accent - 1][0];
    result.hi_blend = accent_colors[accent - 1][1];
  }
  return result;
}

const t_combo_key hotkey_list[] = {
  {"L+R+Start",     0x00F7},
  {"L+R+Select",    0x00FB},
  {"L+R+Start+Sel", 0x00F3},
  {"L+R",           0x00FF},
  {"L+R+A",         0x00FE},
  {"L+R+B",         0x00FD},
  {"L+R+⯇+A",       0x00DE},
  {"L+R+⯈+B",       0x00ED},
  {"L+R+⯅+A",       0x00BE},
  {"L+R+⯆+A",       0x007E},
  {"A+B+Start",     0x03F4},
  {"A+B+Select",    0x03F8},
  {"A+B+Start+Sel", 0x03F0},
};

const char *save_paths[] = { SAVEGAME_DIR, SAVES_DIR };
const char *savestates_paths[] = { SAVESTATE_DIR, "/.superfw/savestate/" };
const char *savestates_paths_display[] = { "/SYSTEM/SAVESTATE/", ".superfw/savestate" };

const uint8_t animspd_lut[] = {
  2,    //  8 pix/second
  3,    // 12 pix/second
  6,    // 24 pix/second
  8,    // 32 pix/second
  12,   // 48 pix/second
};

// Menu settings
uint8_t menu_theme = THEME_DSSMM_LIGHT;
uint8_t lang_id = 0;
uint8_t recent_menu = 1;
uint8_t hide_hidden = 0;
uint8_t anim_speed = animspd_cnt / 2;

// Default settings
t_patch_policy patcher_default = PatchAuto;

uint8_t boot_bios_splash = 0;   // Whether the BIOS boots to the splash screen
uint8_t use_slowld = 0;         // Use slow mirrors for ROM loading, check loaded data.
uint8_t use_fastew = 0;         // Overclock EWRAM while playing.
uint8_t use_verify_nor = 0;     // Verify flash writes

uint8_t save_path_default = SaveSavegameDir;
uint8_t save_path_nor_default = SaveSavegameDir;
uint8_t state_path_default = StateSavestateDir;

uint8_t backup_sram_default = 0;  // Number of older SRAM save to keep as backup

uint8_t hotkey_combo = 0;  // Hotkey Combo number
uint8_t enable_cheats = 0; // By default cheats are disabled (it's slightly faster)

uint8_t autoload_default = 1;
uint8_t autosave_default = 1;
uint8_t autosave_prefer_ds = 1;
uint8_t ingamemenu_default = 1;
uint8_t rtcpatch_default = 1;
uint8_t rtcspeed_default = 3;

uint32_t rtcvalue_default = 45568800U;

// Setting loading/saving routines
bool save_ui_settings() {
  // Create the directory (just in case it doesn't exist)
  f_mkdir(SUPERFW_DIR);

  // Proceed to create the file
  FIL fd;
  if (FR_OK != f_open(&fd, UISETTINGS_FILEPATH, FA_WRITE | FA_CREATE_ALWAYS))
    return false;

  // Serialize the settings
  uint16_t lc = lang_getcode();
  char buf[512];
  npf_snprintf(buf, sizeof(buf),
    "ui_version=%u\n"
    "theme=%u\n"
    "accent=%u\n"
    "artwork=%u\n"
    "langcode=%c%c\n"
    "recent_menu=%u\n"
    "anim_speed=%u\n"
    "hide_hidden=%u\n",
    UI_SETTINGS_VERSION, menu_theme, menu_accent, artwork_mode,
    (lc & 0xFF), (lc >> 8), recent_menu, anim_speed, hide_hidden);

  UINT wrbytes;
  FRESULT res = f_write(&fd, buf, strlen(buf), &wrbytes);
  f_close(&fd);

  return FR_OK == res;
}

bool save_settings() {
  // Create the directory (just in case it doesn't exist)
  f_mkdir(SUPERFW_DIR);

  // Proceed to create the file
  FIL fd;
  if (FR_OK != f_open(&fd, SETTINGS_FILEPATH, FA_WRITE | FA_CREATE_ALWAYS))
    return false;

  // Serialize the settings
  char buf[512];
  npf_snprintf(buf, sizeof(buf),
    "hotkey_opt=%u\n"
    "boot_to_bios=%u\n"
    "save_path_policy=%u\n"
    "save_path_nor_policy=%u\n"
    "state_path_policy=%u\n"
    "sram_backup_count=%u\n"
    "enable_cheats=%u\n"
    "enable_slowld=%u\n"
    "enable_fastewram=%u\n"
    "enable_norwrcheck=%u\n"
    "default_patcher=%u\n"
    "default_igmenu=%u\n"
    "default_rtcpatch=%u\n"
    "default_rtctick=%u\n"
    "default_loadgame=%u\n"
    "default_savegame=%u\n"
    "prefer_directsave=%u\n"
    "default_rtcts=%lu\n",
    hotkey_combo, boot_bios_splash, save_path_default, save_path_nor_default,
    state_path_default, backup_sram_default, enable_cheats, use_slowld, use_fastew,
    use_verify_nor, (unsigned int)patcher_default, ingamemenu_default, rtcpatch_default,
    rtcspeed_default, autoload_default, autosave_default, autosave_prefer_ds,
    rtcvalue_default);

  UINT wrbytes;
  FRESULT res = f_write(&fd, buf, strlen(buf), &wrbytes);
  f_close(&fd);

  return FR_OK == res;
}

static void parse_settings(void *usr, const char *var, const char *value) {
  unsigned valu = parseuint(value);
  if (!strcmp(var, "default_rtcts"))
    rtcvalue_default = valu;
  else {
    static const struct {
      const char *s;
      uint8_t * const var;
    } bolset[] = {
      { "boot_to_bios",      &boot_bios_splash },
      { "enable_cheats",     &enable_cheats },
      { "default_igmenu",    &ingamemenu_default },
      { "enable_slowld",     &use_slowld },
      { "enable_fastewram",  &use_fastew },
      { "enable_norwrcheck", &use_verify_nor },
      { "default_rtcpatch",  &rtcpatch_default },
      { "default_loadgame",  &autoload_default },
      { "default_savegame",  &autosave_default },
      { "prefer_directsave", &autosave_prefer_ds },
    };
    for (unsigned i = 0; i < sizeof(bolset)/sizeof(bolset[0]); i++)
      if (!strcmp(var, bolset[i].s)) {
        *bolset[i].var = valu & 1;
        break;
      }

    static const struct {
      const char *s;
      uint8_t * const var;
      const unsigned modval;
    } uintset[] = {
      { "hotkey_opt",           &hotkey_combo,          sizeof(hotkey_list)/sizeof(hotkey_list[0]) },
      { "save_path_policy",     &save_path_default,     SaveDirCNT },
      { "save_path_nor_policy", &save_path_nor_default, SaveDirNORCNT },
      { "state_path_policy",    &state_path_default,    StateDirCNT },
      { "sram_backup_count",    &backup_sram_default,   MAX_BACKUP_CNT + 1 },
      { "default_patcher",      &patcher_default,       PatchTotalCNT },
      { "default_rtctick",      &rtcspeed_default,      RTC_SPEED_CNT },
    };
    for (unsigned i = 0; i < sizeof(uintset)/sizeof(uintset[0]); i++)
      if (!strcmp(var, uintset[i].s)) {
        *uintset[i].var = valu % uintset[i].modval;
        break;
      }
  }
}

static void parse_ui_settings(void *usr, const char *var, const char *value) {
  if (!strcmp(var, "ui_version")) {
    ui_settings_version = parseuint(value);
  } else if (!strcmp(var, "langcode")) {
    uint16_t code = ((uint8_t)value[0]) | (((uint8_t)value[1]) << 8);
    lang_id = lang_lookup(code);
  } else {
    static const struct {
      const char *s;
      uint8_t * const var;
      unsigned maxval;
    } uintset[] = {
      { "theme",       &menu_theme,  THEME_COUNT },
      { "accent",      &menu_accent, ACCENT_COUNT },
      { "artwork",     &artwork_mode, 3 },
      { "recent_menu", &recent_menu, 2 },
      { "hide_hidden", &hide_hidden, 2 },
      { "anim_speed",  &anim_speed,  animspd_cnt },
    };
    unsigned valu = parseuint(value);
    for (unsigned i = 0; i < sizeof(uintset)/sizeof(uintset[0]); i++)
      if (!strcmp(var, uintset[i].s)) {
        *uintset[i].var = valu < uintset[i].maxval ? valu : 0;
        break;
      }
  }
}

// Parses a config file and calls the user callback with varname+value
// Pointers are only valid for the duration of the callback!
static void parse_file(char *buf, void(*parse_cb)(void *usr, const char*, const char*), void *usrptr) {
  char *p = buf;
  while (1) {
    char *e = strchr(p, '\n');
    if (e)
      *e = 0;

    char *a = strchr(p, '=');
    if (a) {
      *a = 0;
      parse_cb(usrptr, p, &a[1]);
      *a = '=';
    }

    if (!e)
      break;

    *e = '\n';
    p = &e[1];  // Advance to the next line
  }
}

static void migrate_ui_settings() {
  /* Test 1 and earlier settings had no version. Promote their native defaults
   * on every board while preserving explicit choices saved by Test 2 (v2). */
  if (ui_settings_version < UI_SETTINGS_DSSMM_DEFAULTS_VERSION) {
    if (menu_theme == 0)
      menu_theme = THEME_DSSMM_LIGHT;
    if (artwork_mode == 0)
      artwork_mode = 1;
  }
}

void load_settings() {
  FIL fd;
  UINT rdbytes;
  char buf[512];
  ui_settings_version = 0;
  if (FR_OK == f_open(&fd, SETTINGS_FILEPATH, FA_READ)) {
    if (FR_OK == f_read(&fd, buf, sizeof(buf) - 1, &rdbytes)) {
      buf[rdbytes] = 0;
      parse_file(buf, parse_settings, NULL);
    }
    f_close(&fd);
  }

  if (FR_OK == f_open(&fd, UISETTINGS_FILEPATH, FA_READ)) {
    if (FR_OK == f_read(&fd, buf, sizeof(buf) - 1, &rdbytes)) {
      buf[rdbytes] = 0;
      parse_file(buf, parse_ui_settings, NULL);
    }
    f_close(&fd);
  }
  migrate_ui_settings();
}

static bool save_pattern_has_files(const char *pattern) {
  char path[MAX_FN_LEN];
  if (npf_snprintf(path, sizeof(path), "%s.sav", pattern) < (int)sizeof(path) && check_file_exists(path))
    return true;
  if (npf_snprintf(path, sizeof(path), "%s.tmp.sav", pattern) < (int)sizeof(path) && check_file_exists(path))
    return true;
  for (unsigned i = 1; i <= MAX_BACKUP_CNT; ++i)
    if (npf_snprintf(path, sizeof(path), "%s.%u.sav", pattern, i) < (int)sizeof(path) && check_file_exists(path))
      return true;
  return false;
}

static bool make_file_pattern(char *pattern, unsigned size, const char *path, const char *rom) {
  int n = npf_snprintf(pattern, size, "%s%s", path, file_basename(rom));
  if (n < 0 || (unsigned)n >= size)
    return false;
  replace_extension(pattern, "");
  return true;
}

void sram_filename_calc(const char *rom, char *savefn, unsigned save_path) {
  if (save_path == SaveRomName) {
    strbcpy(savefn, rom, MAX_FN_LEN);
  } else {
    char pattern[MAX_FN_LEN];
    if (!make_file_pattern(pattern, sizeof(pattern), save_paths[save_path], rom)) {
      savefn[0] = 0;
      return;
    }
    /* Keep existing root save sets in place and usable until manually migrated. */
    if (!save_pattern_has_files(pattern) && save_path < 2) {
      static const char * const legacy_paths[] = { LEGACY_SAVEGAME_DIR, LEGACY_SAVES_DIR };
      char legacy[MAX_FN_LEN];
      if (make_file_pattern(legacy, sizeof(legacy), legacy_paths[save_path], rom) &&
          save_pattern_has_files(legacy))
        strbcpy(pattern, legacy, sizeof(pattern));
    }
    strbcpy(savefn, pattern, MAX_FN_LEN);
  }
  if (savefn[0])
    replace_extension(savefn, ".sav");
}

void savestate_filename_calc(const char *rom, char *statefn) {
  char pattern[MAX_FN_LEN];
  if (!make_file_pattern(pattern, sizeof(pattern), savestates_paths[state_path_default], rom)) {
    statefn[0] = 0;
    return;
  }
  if (state_path_default == StateSavestateDir) {
    char path[MAX_FN_LEN];
    bool current_exists = false, legacy_exists = false;
    for (unsigned i = 1; i <= DISK_SAVESTATE_SLOTS; ++i)
      if (npf_snprintf(path, sizeof(path), "%s.%u.state", pattern, i) < (int)sizeof(path))
        current_exists |= check_file_exists(path);
    if (!current_exists && make_file_pattern(path, sizeof(path), LEGACY_SAVESTATE_DIR, rom)) {
      char candidate[MAX_FN_LEN];
      for (unsigned i = 1; i <= DISK_SAVESTATE_SLOTS; ++i)
        if (npf_snprintf(candidate, sizeof(candidate), "%s.%u.state", path, i) < (int)sizeof(candidate))
          legacy_exists |= check_file_exists(candidate);
      if (legacy_exists)
        strbcpy(pattern, path, sizeof(pattern));
    }
  }
  strbcpy(statefn, pattern, MAX_FN_LEN);
}

static void parse_rom_load_settings(void *usr, const char *var, const char *value) {
  t_rom_load_settings *rs = (t_rom_load_settings*)usr;
  unsigned valu = parseuint(value);
  if (!strcmp(var, "rtc"))
    rs->use_rtc = valu & 1;
  else if (!strcmp(var, "igm"))
    rs->use_igm = valu & 1;
  else if (!strcmp(var, "directsaving"))
    rs->use_dsaving = valu & 1;
  else if (!strcmp(var, "patchmode"))
    rs->patch_policy = valu % PatchOptCNT;
}

static void parse_rom_launch_settings(void *usr, const char *var, const char *value) {
  t_rom_launch_settings *rs = (t_rom_launch_settings*)usr;
  unsigned valu = parseuint(value);
  if (!strcmp(var, "cheats"))
    rs->use_cheats = valu & 1;
  else if (!strcmp(var, "rtcts"))
    rs->rtcts = valu;
}

bool load_rom_settings(const char *fn, t_rom_load_settings *rld, t_rom_launch_settings *rlh) {
  FIL fd;
  {
    char path[MAX_FN_LEN];
    strcpy(path, ROMCONFIG_PATH);
    strcat(path, file_basename(fn));
    replace_extension(path, ".config");

    // Attempt to open and read the file.
    if (FR_OK != f_open(&fd, path, FA_READ))
      return false;
  }

  {
    char buf[128];
    UINT rdbytes;
    if (FR_OK == f_read(&fd, buf, sizeof(buf) - 1, &rdbytes)) {
      buf[rdbytes] = 0;
      if (rld)
        parse_file(buf, parse_rom_load_settings, rld);
      if (rlh)
        parse_file(buf, parse_rom_launch_settings, rlh);
    }
  }
  f_close(&fd);

  return true;
}

bool save_rom_settings(const char *fn, const t_rom_load_settings *rld, const t_rom_launch_settings *rlh) {
  // Create the directory (just in case it doesn't exist)
  f_mkdir(SUPERFW_DIR);
  f_mkdir(ROMCONFIG_PATH);

  // Proceed to create the file
  FIL fd;
  {
    char path[MAX_FN_LEN];
    strcpy(path, ROMCONFIG_PATH);
    strcat(path, file_basename(fn));
    replace_extension(path, ".config");

    if (FR_OK != f_open(&fd, path, FA_WRITE | FA_CREATE_ALWAYS))
      return false;
  }

  // Serialize the ROM settings
  FRESULT res;
  {
    char buf[128];
    npf_snprintf(buf, sizeof(buf),
      "patchmode=%u\n"
      "igm=%u\n"
      "rtc=%u\n"
      "directsaving=%u\n"
      "cheats=%u\n"
      "rtcts=%u\n",
      rld->patch_policy,
      rld->use_igm ? 1 : 0,
      rld->use_rtc ? 1 : 0,
      rld->use_dsaving ? 1 : 0,
      rlh->use_cheats ? 1 : 0,
      (unsigned int)rlh->rtcts);

    UINT wrbytes;
    res = f_write(&fd, buf, strlen(buf), &wrbytes);
    f_close(&fd);
  }

  return FR_OK == res;
}


