/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "posix-fatfs-fixture.h"
volatile unsigned frame_count;
#include "../src/settings.c"
unsigned lang_lookup(uint16_t code){(void)code;return 0;}
uint16_t lang_getcode(void){return 0x6e65;}
int main(void) {
 char tmp[]="/tmp/dssmm-settings-roundtrip-XXXXXX";assert(mkdtemp(tmp));strcpy(root,tmp);assert(f_mkdir("/SYSTEM")==FR_OK);
 write_file(SETTINGS_FILEPATH,"\xef\xbb\xbfTheme = Light\r\nColour = Pale Blue\r\nView mode = Horizontal\r\nThumbnails = Title\r\nUnknown DE = retain\r\n");
 write_file(UISETTINGS_FILEPATH,"ui_version=8\ntheme=6\naccent=1\nartwork=1\nbrowser_view=1\nfont=1\n");
 load_settings();assert(menu_theme==6 && !menu_accent && browser_view==1 && artwork_mode==1 && menu_font==1);
 for(unsigned theme=0;theme<8;theme++)for(unsigned accent=0;accent<5;accent++)
 for(unsigned view=0;view<3;view++)for(unsigned art=0;art<3;art++) {
  menu_theme=theme;menu_accent=accent;browser_view=view;artwork_mode=art;
  assert(save_settings());
  menu_theme=0;menu_accent=0;browser_view=0;artwork_mode=0;load_settings();
  if(menu_theme!=theme || menu_accent!=accent || browser_view!=view || artwork_mode!=art)fprintf(stderr,"requested %u %u %u %u restored %u %u %u %u\n",theme,accent,view,art,menu_theme,menu_accent,browser_view,artwork_mode);
  assert(menu_theme==theme && menu_accent==accent && browser_view==view && artwork_mode==art);
 }
 char text[8192];read_file(SETTINGS_FILEPATH,text,sizeof(text));assert(strstr(text,"Unknown DE = retain"));assert(f_stat(UISETTINGS_FILEPATH,0)==FR_NO_FILE);
 read_error=1;load_settings();assert(!settings_loaded_ok());assert(!save_settings());read_error=0;
 load_settings();assert(settings_loaded_ok());
 assert(f_unlink(SETTINGS_FILEPATH)==FR_OK);char q[1024];path(q,"/SYSTEM");assert(!rmdir(q));assert(!rmdir(root));
 puts("Unified settings: all theme/accent/view/artwork combinations survive real saves and reloads; legacy sidecar retired safely.");
}
