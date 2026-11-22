#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "${MENU_SOURCE:-$root/src/menu.c}" "$tmp/test.c" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text();start=s.index('    // Move menu up and down',s.index('static void keypress_menu_browse('));end=s.index('    // Move into a new dir',start)
Path(sys.argv[2]).write_text('''#include <assert.h>
#include "browser_carousel.h"
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define KEY_BUTTUP 1
#define KEY_BUTTDOWN 2
#define KEY_BUTTLEFT 4
#define KEY_BUTTRIGHT 8
#define BROWSER_ROWS 8
static int browser_view;
static int carousel_wrap;
static struct {struct {int selector,seloff,dispentries;} browser;} smenu;
static void navigate(unsigned newkeys) {
if(smenu.browser.dispentries) {
'''+s[start:end]+'''}}
int main(void) {
for(carousel_wrap=0;carousel_wrap<2;++carousel_wrap)
for(browser_view=0;browser_view<3;++browser_view)
 for(int count=0;count<20;++count)
  for(int selected=0;selected<(count?count:1);++selected)
   for(unsigned key=1;key<=8;key*=2) {
    smenu.browser.dispentries=count;smenu.browser.selector=selected;smenu.browser.seloff=selected;
    navigate(key);
    assert(smenu.browser.selector>=0);
    assert(smenu.browser.selector<(count?count:1));
    assert(smenu.browser.seloff>=0);
   }
browser_view=1;carousel_wrap=1;smenu.browser.dispentries=16;smenu.browser.selector=0;navigate(KEY_BUTTLEFT);assert(smenu.browser.selector==15);navigate(KEY_BUTTRIGHT);assert(!smenu.browser.selector);
return 0;}
''')
PY
gcc -Wall -Wextra -Werror -Wno-sign-compare -Wno-type-limits -I "$root/src" "$tmp/test.c" -o "$tmp/test"
"$tmp/test"
printf 'Production browser edge navigation passed for List/Horizontal/Vertical.\n'
