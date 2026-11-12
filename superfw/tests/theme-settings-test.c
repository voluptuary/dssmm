#include <assert.h>
#include "../src/settings.c"
unsigned lang_lookup(uint16_t code) { (void)code; return 0; }
uint16_t lang_getcode(void) { return 0x6e65; }
int main(void) {
#ifdef SUPERCHIS_IO
  assert(menu_theme == THEME_DSSMM_LIGHT);
#else
  assert(menu_theme == 0);
#endif
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
  return 0;
}
