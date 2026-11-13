#include <assert.h>
#include "../src/settings.c"
unsigned lang_lookup(uint16_t code) { (void)code; return 0; }
uint16_t lang_getcode(void) { return 0x6e65; }
static char saved_settings[512];
static const char *existing_path;
FRESULT f_stat(const TCHAR *path, FILINFO *info) {
  (void)info;
  return existing_path && !strcmp(path, existing_path) ? FR_OK : FR_NO_FILE;
}
FRESULT f_mkdir(const TCHAR *path) { (void)path; return FR_OK; }
FRESULT f_open(FIL *fp, const TCHAR *path, BYTE mode) {
  (void)fp; (void)path; (void)mode; return FR_OK;
}
FRESULT f_close(FIL *fp) { (void)fp; return FR_OK; }
FRESULT f_write(FIL *fp, const void *data, UINT size, UINT *written) {
  (void)fp; assert(size < sizeof(saved_settings));
  memcpy(saved_settings, data, size); saved_settings[size] = 0;
  *written = size; return FR_OK;
}
int main(void) {
  assert(menu_theme == THEME_DSSMM_LIGHT);
  assert(artwork_mode == 1);
  assert(THEME_DSSMM_LIGHT == 6 && THEME_DSSMM_DARK == 7);
  const char *values[] = {"0", "1", "2", "3", "4", "5", "6", "7"};
  for (unsigned i = 0; i < THEME_COUNT; ++i) {
    parse_ui_settings(0, "theme", values[i]);
    assert(menu_theme == i);
    assert(themes[i].bg_color != themes[i].ft_color);
  }
  parse_ui_settings(0, "theme", "8"); assert(menu_theme == 0);
  parse_ui_settings(0, "theme", "255"); assert(menu_theme == 0);
  assert(themes[0].bg_color == RGB2GBA(0xffffff));
  assert(themes[5].bg_color == RGB2GBA(0x444444));
  assert(menu_accent == 0);
  for (unsigned theme = 0; theme < THEME_COUNT; ++theme) {
    t_theme_colors original = menu_theme_colors(theme, 0);
    assert(memcmp(&original, &themes[theme], sizeof(original)) == 0);
    for (unsigned accent = 1; accent < ACCENT_COUNT; ++accent) {
      t_theme_colors colors = menu_theme_colors(theme, accent);
      assert(colors.bg_color == original.bg_color);
      assert(colors.ft_color == original.ft_color);
      assert(colors.fg_color == original.fg_color);
      assert(colors.sh_color == original.sh_color);
      assert(colors.hi_color != colors.bg_color);
    }
  }
  t_theme_colors fallback = menu_theme_colors(255, 255);
  assert(memcmp(&fallback, &themes[0], sizeof(fallback)) == 0);
  parse_ui_settings(0, "accent", "4"); assert(menu_accent == 4);
  parse_ui_settings(0, "accent", "5"); assert(menu_accent == 0);
  parse_ui_settings(0, "artwork", "2"); assert(artwork_mode == 2);
  parse_ui_settings(0, "artwork", "3"); assert(artwork_mode == 0);
  ui_settings_version = 0; menu_theme = 0; artwork_mode = 0; migrate_ui_settings();
  assert(menu_theme == THEME_DSSMM_LIGHT && artwork_mode == 1);
  ui_settings_version = 1; menu_theme = 0; artwork_mode = 0; migrate_ui_settings();
  assert(menu_theme == THEME_DSSMM_LIGHT && artwork_mode == 1);
  parse_ui_settings(0, "ui_version", "2"); menu_theme = 0; artwork_mode = 0; migrate_ui_settings();
  assert(menu_theme == 0 && artwork_mode == 0); /* Test 2 explicit choices survive. */
  parse_ui_settings(0, "ui_version", "3"); menu_theme = 0; artwork_mode = 0; migrate_ui_settings();
  assert(menu_theme == 0 && artwork_mode == 0);

  assert(!strcmp(save_paths[0], "/SYSTEM/SAVEGAME/"));
  assert(!strcmp(save_paths[1], "/SYSTEM/SAVES/"));
  existing_path = "/SAVEGAME/Game.sav";
  char path[MAX_FN_LEN];
  sram_filename_calc("/Games/Game.gba", path, SaveSavegameDir);
  assert(!strcmp(path, "/SAVEGAME/Game.sav"));
  existing_path = NULL;
  sram_filename_calc("/Games/Game.gba", path, SaveSavegameDir);
  assert(!strcmp(path, "/SYSTEM/SAVEGAME/Game.sav"));
  existing_path = "/SAVESTATE/Game.2.state";
  state_path_default = StateSavestateDir;
  savestate_filename_calc("/Games/Game.gba", path);
  assert(!strcmp(path, "/SAVESTATE/Game"));
  existing_path = NULL;
  savestate_filename_calc("/Games/Game.gba", path);
  assert(!strcmp(path, "/SYSTEM/SAVESTATE/Game"));
  menu_theme = THEME_DSSMM_DARK; menu_accent = 3; artwork_mode = 2;
  assert(save_ui_settings());
  assert(strstr(saved_settings, "ui_version=3\n"));
  assert(strstr(saved_settings, "theme=7\naccent=3\n"));
  menu_theme = 0; menu_accent = 0; artwork_mode = 0;
  parse_file(saved_settings, parse_ui_settings, 0);
  assert(menu_theme == THEME_DSSMM_DARK && menu_accent == 3 && artwork_mode == 2);
  return 0;
}
