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
#include <strings.h>
#include "settings_io.h"
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
   * The native palette roles remain stable alongside credited DE backgrounds.
   * Accents stay readable with the renderer's existing black/white UI masks. */
  [THEME_DSSMM_LIGHT] = { RGB2GBA(0xe0e0e0), RGB2GBA(0xffffff), RGB2GBA(0x000000), RGB2GBA(0x80b8e8), RGB2GBA(0x90b8d8), RGB2GBA(0x788088) },
  [THEME_DSSMM_DARK] = { RGB2GBA(0x181818), RGB2GBA(0x000000), RGB2GBA(0xffffff), RGB2GBA(0x5088b8), RGB2GBA(0x406080), RGB2GBA(0x889098) },
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
char menu_username[LAUNCHER_NAME_BYTES];
uint32_t menu_clock_seed=UINT32_MAX;
uint8_t menu_font = FONT_STYLE_NATIVE;
uint8_t clean_list=0,hide_system_files=1,menu_clock_24=1;
static uint8_t thumbnail_preference=1;
static bool artwork_disabled;
uint8_t artwork_mode = 1;
#define UI_SETTINGS_VERSION 10
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
uint8_t browser_view = 0;

// Default settings
t_patch_policy patcher_default = PatchAuto;

uint8_t boot_bios_splash = 0;   // Whether the BIOS boots to the splash screen
uint8_t use_slowld = 0;         // Use slow mirrors for ROM loading, check loaded data.
uint8_t use_fastew = 0;         // Overclock EWRAM while playing.
uint8_t use_verify_nor = 0;     // Verify flash writes


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

/* DE-compatible keys precede kernel-specific values in one portable file. */
enum {SharedTheme,SharedColour,SharedHidden,SharedClean,SharedView,SharedArt,SharedLanguage,SharedClock,SharedCount};
static const char * const shared_keys[]={"Theme","Colour","Hide system files","Clean list","View mode","Thumbnails","Language","Clock format"};
/* Cold text cache belongs in EWRAM; retain IWRAM for rendering and stack. */
static struct {char value[64];unsigned baseline;bool long_value;} shared_values[SharedCount]
  __attribute__((section(".sbss"),aligned(4)));
static bool shared_seen,unified_seen;
static int shared_view=-1;
static unsigned shared_current(unsigned id) {
  switch(id) {
    case SharedTheme:return menu_theme==THEME_DSSMM_DARK || menu_theme==5;
    case SharedColour:return menu_accent;
    case SharedHidden:return hide_system_files;
    case SharedClean:return clean_list;
    case SharedView:return browser_view?browser_view:artwork_mode?3:0;
    case SharedArt:return artwork_mode?artwork_mode:thumbnail_preference;
    case SharedLanguage:return lang_getcode();
    default:return menu_clock_24;
  }
}
static const char *shared_value(unsigned id,const char *fallback) {
  return shared_values[id].value[0] && shared_current(id)==shared_values[id].baseline?shared_values[id].value:fallback;
}
static const struct {const char *name,*code;} language_names[]={
  {"English (US)","en"},{"English (UK)","en"},{"Français","fr"},{"Español","es"},{"Deutsch","de"},
  {"Português","pt"},{"Italiano","it"},{"Nederlands","nl"},{"Svenska","sv"},{"Suomi","fi"},
  {"Türkçe","tr"},{"Chinese","zh"},{"Japanese","ja"},{"Korean","ko"},{"Polski","pl"},{"Russian","ru"}
};
static const char *language_name(void) {
  unsigned code=lang_getcode();
  for(unsigned i=0;i<sizeof(language_names)/sizeof(*language_names);++i)
    if(code==((uint8_t)language_names[i].code[0]|((uint8_t)language_names[i].code[1]<<8)))return language_names[i].name;
  return "English (US)";
}
static const char * const native_keys[]={"artwork_off","ui_version","theme","native_theme","font","clock_seconds","accent","artwork","browser_view","langcode","recent_menu","anim_speed","hide_hidden","hotkey_opt","boot_to_bios","sram_backup_count","enable_cheats","enable_slowld","enable_fastewram","enable_norwrcheck","default_patcher","default_igmenu","default_rtcpatch","default_rtctick","default_loadgame","default_savegame","prefer_directsave","default_rtcts"};
static bool settings_owned(const char *key) {
  for(unsigned i=0;i<SharedCount;++i)if(!strcasecmp(key,shared_keys[i]))
    return !(shared_values[i].long_value && shared_current(i)==shared_values[i].baseline);
  if(!strncasecmp(key,"SuperFW.",8))key+=8;
  for(unsigned i=0;i<sizeof(native_keys)/sizeof(*native_keys);++i)if(!strcmp(key,native_keys[i]))return true;
  return false;
}
bool save_settings(void) {
  menu_clock_seed=launcher_clock_get();
  const char * const colors[]={"Pale Blue","Blue","Green","Orange","Purple"};
  const char * const views[]={"List","Horizontal","Vertical","List + art"};
  const char *values[]={menu_theme==THEME_DSSMM_DARK || menu_theme==5?"Dark":"Light",colors[menu_accent<5?menu_accent:0],
    hide_system_files?"On":"Off",clean_list?"On":"Off",views[shared_current(SharedView)],
    shared_current(SharedArt)==2?"Box":"Title",language_name(),menu_clock_24?"24 hour":"12 hour"};
  char common[1536];unsigned used=0;
  for(unsigned i=0;i<SharedCount;++i) {
    if(shared_values[i].long_value && shared_current(i)==shared_values[i].baseline)continue;
    int n=npf_snprintf(common+used,sizeof(common)-used,"%s = %s\n",shared_keys[i],shared_value(i,values[i]));
    if(n<0 || (unsigned)n>=sizeof(common)-used)return false;
    used+=n;
  }
  char extra[1536];
  int n=npf_snprintf(extra,sizeof(extra),
    "SuperFW.ui_version=%u\nSuperFW.native_theme=%u\nSuperFW.font=%u\nSuperFW.artwork_off=%u\nSuperFW.clock_seconds=%lu\n"
    "SuperFW.recent_menu=%u\nSuperFW.anim_speed=%u\nSuperFW.hide_hidden=%u\n"
    "SuperFW.hotkey_opt=%u\nSuperFW.boot_to_bios=%u\nSuperFW.sram_backup_count=%u\nSuperFW.enable_cheats=%u\n"
    "SuperFW.enable_slowld=%u\nSuperFW.enable_fastewram=%u\nSuperFW.enable_norwrcheck=%u\n"
    "SuperFW.default_patcher=%u\nSuperFW.default_igmenu=%u\nSuperFW.default_rtcpatch=%u\nSuperFW.default_rtctick=%u\n"
    "SuperFW.default_loadgame=%u\nSuperFW.default_savegame=%u\nSuperFW.prefer_directsave=%u\nSuperFW.default_rtcts=%lu\n",
    UI_SETTINGS_VERSION,menu_theme<6?menu_theme:255,menu_font,!artwork_mode,(unsigned long)menu_clock_seed,recent_menu,anim_speed,hide_hidden,
    hotkey_combo,boot_bios_splash,backup_sram_default,enable_cheats,use_slowld,use_fastew,use_verify_nor,
    (unsigned)patcher_default,ingamemenu_default,rtcpatch_default,rtcspeed_default,autoload_default,autosave_default,autosave_prefer_ds,(unsigned long)rtcvalue_default);
  if(n<0 || (unsigned)n>=sizeof(extra))return false;
  return settings_write_text(common,extra,settings_owned);
}
bool save_ui_settings(void) {return save_settings();}

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
  if (!strcmp(var,"clock_seconds")) {
    uint32_t seconds=parseuint(value);
    menu_clock_seed=seconds<86400?seconds:UINT32_MAX;
  } else if (!strcmp(var, "ui_version")) {
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
      { "font",&menu_font, FONT_STYLE_COUNT },
      { "accent",      &menu_accent, ACCENT_COUNT },
      { "artwork",     &artwork_mode, 3 },
      { "browser_view",&browser_view, 3 },
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

static bool setting_on(const char *value) {return !strcasecmp(value,"On") || !strcasecmp(value,"Yes") || !strcmp(value,"1");}
static void read_setting(const char *key,const char *value) {
  if(!strncasecmp(key,"SuperFW.",8)) {
    key+=8;
    if(!strcmp(key,"ui_version"))unified_seen=true;
    if(!strcmp(key,"artwork_off")){artwork_disabled=setting_on(value);return;}
    if(!strcmp(key,"native_theme")) {unsigned n=parseuint(value);if(n<6)menu_theme=n;return;}
    parse_settings(0,key,value);parse_ui_settings(0,key,value);return;
  }
  /* Old lowercase numeric theme is a legacy SuperFW preference. */
  if(!strcmp(key,"theme") && *value>='0' && *value<='9'){parse_ui_settings(0,key,value);return;}
  for(unsigned i=0;i<SharedCount;++i)if(!strcasecmp(key,shared_keys[i])) {
    shared_seen=true;shared_values[i].long_value=strlen(value)>=sizeof(shared_values[i].value);
    strbcpy(shared_values[i].value,value,sizeof(shared_values[i].value));
    switch(i) {
      case SharedTheme:menu_theme=!strcasecmp(value,"Dark")?THEME_DSSMM_DARK:THEME_DSSMM_LIGHT;break;
      case SharedColour:{static const char *const names[]={"Pale Blue","Blue","Green","Orange","Purple"};menu_accent=0;for(unsigned c=0;c<5;++c)if(!strcasecmp(value,names[c]))menu_accent=c;break;}
      case SharedHidden:hide_system_files=setting_on(value);break;
      case SharedClean:clean_list=setting_on(value);break;
      case SharedView:shared_view=!strcasecmp(value,"Horizontal")?1:!strcasecmp(value,"Vertical")?2:!strcasecmp(value,"List + art")||!strcasecmp(value,"List art")?3:0;break;
      case SharedArt:thumbnail_preference=!strcasecmp(value,"Box")?2:1;break;
      case SharedClock:menu_clock_24=strcasecmp(value,"12 hour") && strcasecmp(value,"12-hour") && strcasecmp(value,"12H");break;
      case SharedLanguage:for(unsigned l=0;l<sizeof(language_names)/sizeof(*language_names);++l)if(!strcasecmp(value,language_names[l].name)){lang_id=lang_lookup((uint8_t)language_names[l].code[0]|((uint8_t)language_names[l].code[1]<<8));break;}break;
    }
    return;
  }
  parse_settings(0,key,value);parse_ui_settings(0,key,value);
}
static void probe_setting(const char *key,const char *value) {(void)value;if(!strcasecmp(key,"SuperFW.ui_version"))unified_seen=true;}
void load_settings(void) {
  ui_settings_version=0;
  bool exists;artwork_disabled=false;unified_seen=shared_seen=false;shared_view=-1;memset(shared_values,0,sizeof(shared_values));
  settings_read_text(SETTINGS_FILEPATH,probe_setting,&exists);
  if(!unified_seen)settings_read_text(UISETTINGS_FILEPATH,read_setting,&exists);
  settings_read_text(SETTINGS_FILEPATH,read_setting,&exists);
  if(shared_view>=0){browser_view=shared_view==3?0:shared_view;artwork_mode=shared_view?thumbnail_preference:0;}
  else if(artwork_mode)thumbnail_preference=artwork_mode;
  if(artwork_disabled)artwork_mode=0;
  if(shared_seen)ui_settings_version=UI_SETTINGS_VERSION;else migrate_ui_settings();
  for(unsigned i=0;i<SharedCount;++i)shared_values[i].baseline=shared_current(i);
  menu_username[0]=0;FIL fd;UINT size;char name[64];
  if(f_open(&fd,"/SYSTEM/NAME.TXT",FA_READ)==FR_OK || f_open(&fd,"/SYSTEM/NAME",FA_READ)==FR_OK) {
    FRESULT read=f_read(&fd,name,sizeof(name),&size),closed=f_close(&fd);
    if(read==FR_OK && closed==FR_OK)launcher_name_parse(menu_username,name,size);
  }
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

/* Preserve saves made by the old double-stripping rule without renaming or
 * overriding a correctly named save. New saves always use the complete stem. */
static bool legacy_shortened_pattern(char *out,const char *full_stem) {
  if(!find_extension(full_stem))return false;
  char shortened[MAX_FN_LEN];strbcpy(shortened,full_stem,sizeof(shortened));
  replace_extension(shortened,"");
  if(!save_pattern_has_files(shortened))return false;
  strbcpy(out,shortened,MAX_FN_LEN);return true;
}

void sram_filename_calc(const char *rom, char *savefn) {
  char pattern[MAX_FN_LEN];
  if (!make_file_pattern(pattern,sizeof(pattern),SAVER_DIR,rom)) {savefn[0]=0;return;}
  if (!save_pattern_has_files(pattern)) {
    static const char * const legacy_paths[]={LEGACY_SYSTEM_SAVEGAME_DIR,LEGACY_SAVEGAME_DIR,SAVES_DIR,LEGACY_SAVES_DIR};
    for(unsigned i=0;i<4;++i) {
      char candidate[MAX_FN_LEN];
      if(make_file_pattern(candidate,sizeof(candidate),legacy_paths[i],rom) && save_pattern_has_files(candidate)) {strbcpy(pattern,candidate,sizeof(pattern));break;}
    }
    if(!save_pattern_has_files(pattern)) {
      char candidate[MAX_FN_LEN];strbcpy(candidate,rom,sizeof(candidate));replace_extension(candidate,"");
      if(save_pattern_has_files(candidate))strbcpy(pattern,candidate,sizeof(pattern));
    }
  }
  if(!save_pattern_has_files(pattern)) {
    const char * const paths[]={SAVER_DIR,LEGACY_SYSTEM_SAVEGAME_DIR,LEGACY_SAVEGAME_DIR,SAVES_DIR,LEGACY_SAVES_DIR};
    bool found=false;
    for(unsigned i=0;i<sizeof(paths)/sizeof(*paths) && !found;++i) {
      char candidate[MAX_FN_LEN];
      if(make_file_pattern(candidate,sizeof(candidate),paths[i],rom))
        found=legacy_shortened_pattern(pattern,candidate);
    }
    if(!found) {
      char candidate[MAX_FN_LEN];strbcpy(candidate,rom,sizeof(candidate));replace_extension(candidate,"");
      legacy_shortened_pattern(pattern,candidate);
    }
  }
  /* The ROM extension is already removed. Append once: replacing again
   * would remove a real dot in names such as Game.v1.2 (USA).gg. */
  int length=npf_snprintf(savefn,MAX_FN_LEN,"%s.sav",pattern);
  if(length<0 || (unsigned)length>=MAX_FN_LEN)savefn[0]=0;
}

void savestate_filename_calc(const char *rom, char *statefn) {
  char pattern[MAX_FN_LEN];
  if (!make_file_pattern(pattern, sizeof(pattern), SAVESTATE_DIR, rom)) {
    statefn[0] = 0;
    return;
  }
  {
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


