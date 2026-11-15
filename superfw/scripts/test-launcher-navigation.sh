#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$root/src/menu.c" "$tmp/test.c" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
a=s.index('enum {\n  MENUTAB_RECENT');b=s.index('\n};',a)+3
out='''#include <assert.h>
#include <stdint.h>
#define THEME_DSSMM_LIGHT 6
#define KEY_BUTTA 1
#define KEY_BUTTB 2
#define KEY_BUTTSTA 8
#define KEY_BUTTRIGHT 16
#define KEY_BUTTLEFT 32
#define KEY_BUTTUP 64
#define KEY_BUTTDOWN 128
#define MIN(a,b) ((a)<(b)?(a):(b))
#define DefsPatchEng 12
#define SettHotkey 1
static unsigned menu_theme=6,launcher_page=1,launcher_selected=1,launcher_category;
static struct {unsigned menu_tab;struct {int selector,seloff,maxentries;} recent;struct {unsigned selector;} set;} smenu;
static unsigned recent_actions,invalidations;
static void browser_invalidate_artwork(void){++invalidations;}
static void launcher_invalidate(void){}
static void keypress_menu_recent(unsigned keys){assert(keys==KEY_BUTTA);++recent_actions;}
'''+s[a:b]+'\nstatic void press(unsigned newkeys) {\n'
a=s.index('    if(menu_theme>=THEME_DSSMM_LIGHT) {',s.index('void menu_keypress('));b=s.index('    // Menu change via trigger buttons',a)
out+=s[a:b]+'''}
int main(void) {
 press(KEY_BUTTRIGHT);assert(launcher_selected==2);
 press(KEY_BUTTA);assert(launcher_page==0);
#ifdef SUPPORT_NORGAMES
 assert(smenu.menu_tab==MENUTAB_NORBROWSE);
#else
 assert(smenu.menu_tab==MENUTAB_RECENT);
#endif
 press(KEY_BUTTSTA);assert(launcher_page==1);
 press(KEY_BUTTUP);assert(launcher_selected==0);
 press(KEY_BUTTA);assert(!recent_actions && launcher_page==1);
 smenu.recent.maxentries=1;smenu.recent.selector=9;smenu.recent.seloff=4;
 press(KEY_BUTTSTA);assert(recent_actions==1 && launcher_page==1);
 assert(!smenu.recent.selector && !smenu.recent.seloff);
 press(KEY_BUTTDOWN);assert(launcher_selected==1);
 press(KEY_BUTTDOWN);assert(launcher_selected==3);
 press(KEY_BUTTA);assert(launcher_page==2);
 for(unsigned i=0;i<9;++i)press(KEY_BUTTDOWN);
 assert(launcher_category==4);press(KEY_BUTTA);
 assert(!launcher_page && smenu.menu_tab==MENUTAB_INFO);
 press(KEY_BUTTB);assert(launcher_page==2);
 for(unsigned i=0;i<9;++i)press(KEY_BUTTUP);
 assert(!launcher_category);press(KEY_BUTTA);
 assert(!launcher_page && smenu.menu_tab==MENUTAB_UILANG);
 press(KEY_BUTTB);assert(launcher_page==2);
 launcher_category=1;press(KEY_BUTTA);
 assert(!launcher_page && smenu.menu_tab==MENUTAB_SETTINGS && smenu.set.selector==DefsPatchEng);
 press(KEY_BUTTB);launcher_category=2;press(KEY_BUTTA);
 assert(smenu.set.selector==SettHotkey);
 press(KEY_BUTTB);press(KEY_BUTTB);assert(launcher_page==1);
 press(KEY_BUTTB);assert(!launcher_page && smenu.menu_tab==MENUTAB_ROMBROWSE);
 menu_theme=0;press(KEY_BUTTSTA);assert(!launcher_page);
 assert(invalidations);
}
'''
Path(sys.argv[2]).write_text(out)
PY
for board in sd lite chis; do
 flags=();if [ "$board" = chis ];then flags=(-DSUPPORT_NORGAMES);fi
 gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie "${flags[@]}" "$tmp/test.c" -o "$tmp/test"
 "$tmp/test"
done
printf 'Production Start/settings routing, bounds, Last Played, board actions and native-theme isolation passed.\n'
