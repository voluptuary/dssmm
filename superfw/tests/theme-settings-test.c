#include <assert.h>
volatile unsigned frame_count;
#include "../src/settings.c"
unsigned lang_lookup(uint16_t code) { (void)code; return 0; }
uint16_t lang_getcode(void) { return 0x6e65; }
static char saved_settings[512];
static const char *existing_path,*second_existing;
FRESULT f_stat(const TCHAR *path, FILINFO *info) {
  (void)info;
  return ((existing_path && !strcmp(path,existing_path)) || (second_existing && !strcmp(path,second_existing))) ? FR_OK : FR_NO_FILE;
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
  parse_ui_settings(0,"clock_seconds","86399");assert(menu_clock_seed==86399);
  parse_ui_settings(0,"clock_seconds","86400");assert(menu_clock_seed==UINT32_MAX);
  parse_ui_settings(0,"carousel_animation","0"); /* Retired key is ignored. */
  assert(menu_font==FONT_STYLE_NATIVE);
  parse_ui_settings(0,"font","1");assert(menu_font==FONT_STYLE_DSSMM);
  parse_ui_settings(0,"font","99");assert(menu_font==FONT_STYLE_NATIVE);
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
  parse_ui_settings(0, "browser_view", "2"); assert(browser_view == 2);
  parse_ui_settings(0, "browser_view", "3"); assert(browser_view == 0);
  ui_settings_version = 0; menu_theme = 0; artwork_mode = 0; migrate_ui_settings();
  assert(menu_theme == THEME_DSSMM_LIGHT && artwork_mode == 1);
  ui_settings_version = 1; menu_theme = 0; artwork_mode = 0; migrate_ui_settings();
  assert(menu_theme == THEME_DSSMM_LIGHT && artwork_mode == 1);
  parse_ui_settings(0, "ui_version", "2"); menu_theme = 0; artwork_mode = 0; migrate_ui_settings();
  assert(menu_theme == 0 && artwork_mode == 0); /* Test 2 explicit choices survive. */
  parse_ui_settings(0, "ui_version", "4"); menu_theme = 0; artwork_mode = 0; migrate_ui_settings();
  assert(menu_theme == 0 && artwork_mode == 0);

  existing_path = "/SAVEGAME/Game.sav";
  char path[MAX_FN_LEN];
  sram_filename_calc("/Games/Game.gba", path);
  assert(!strcmp(path, "/SAVEGAME/Game.sav"));
  existing_path = NULL;
  sram_filename_calc("/Games/Game.gba", path);
  assert(!strcmp(path, "/SYSTEM/SAVER/Game.sav"));
  existing_path="/SYSTEM/SAVEGAME/Game.sav";
  sram_filename_calc("/Games/Game.gba",path);assert(!strcmp(path,existing_path));
  second_existing=existing_path;existing_path="/SYSTEM/SAVER/Game.sav";
  sram_filename_calc("/Games/Game.gba",path);assert(!strcmp(path,existing_path));second_existing=0;
  existing_path="/SYSTEM/SAVES/Game.sav";
  sram_filename_calc("/Games/Game.gba",path);
  assert(!strcmp(path,"/SYSTEM/SAVES/Game.sav"));
  existing_path="/Games/Game.sav";
  sram_filename_calc("/Games/Game.gba",path);
  assert(!strcmp(path,"/Games/Game.sav"));
  existing_path = "/SAVESTATE/Game.2.state";
  savestate_filename_calc("/Games/Game.gba", path);
  assert(!strcmp(path, "/SAVESTATE/Game"));
  existing_path = NULL;
  parse_settings(0,"save_path_policy","2");
  parse_settings(0,"state_path_policy","1");
  savestate_filename_calc("/Games/Game.gba", path);
  assert(!strcmp(path, "/SYSTEM/SAVESTATE/Game"));
  const char *rom_names[]={"/Games/Adventure (USA).gb","/Game Gear/Sonic (USA).gg",
    "/Sega.v2/Sonic.v1.2 (USA).gg","/Game.Boy/Game.v2.GB"};
  const char *save_names[]={"/SYSTEM/SAVER/Adventure (USA).sav","/SYSTEM/SAVER/Sonic (USA).sav",
    "/SYSTEM/SAVER/Sonic.v1.2 (USA).sav","/SYSTEM/SAVER/Game.v2.sav"};
  for(unsigned i=0;i<4;++i){sram_filename_calc(rom_names[i],path);assert(!strcmp(path,save_names[i]));}
  existing_path="/SYSTEM/SAVER/Adventure (USA).tmp.sav";
  sram_filename_calc("/Games/Adventure (USA).gb",path);
  assert(!strcmp(path,"/SYSTEM/SAVER/Adventure (USA).sav"));
  existing_path="/SAVEGAME/Sonic (USA).2.sav";
  sram_filename_calc("/Game Gear/Sonic (USA).gg",path);
  assert(!strcmp(path,"/SAVEGAME/Sonic (USA).sav"));existing_path=0;
  savestate_filename_calc("/Game.Boy/Game.v2.GB",path);
  assert(!strcmp(path,"/SYSTEM/SAVESTATE/Game.v2"));
  char longrom[256];longrom[0]='/';memset(longrom+1,'a',238);strcpy(longrom+239,".gb");
  sram_filename_calc(longrom,path);assert(!path[0]); /* Reject rather than overrun/truncate. */
  read_setting("Theme","Dark");assert(menu_theme==THEME_DSSMM_DARK);
  read_setting("Clean list","On");assert(clean_list==1);
  read_setting("View mode","Horizontal");assert(shared_view==1);
  return 0;
}
