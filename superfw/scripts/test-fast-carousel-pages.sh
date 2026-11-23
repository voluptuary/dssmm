#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$root/src/menu.c" "$tmp/test.c" <<'PY'
from pathlib import Path
import sys
source=Path(sys.argv[1]).read_text()
a=source.index('  bool fast_carousel =',source.index('void menu_render('))
b=source.index('  // Preserve stable artwork',a)
c=source.index('  if (fast_carousel) {',b)
d=source.index('\n  }',c)+4
helpers=''
for signature in ['static bool carousel_source_nor(', 'static bool carousel_source_active(', 'static int carousel_source_selected(']:
 start=source.index(signature);end=source.index('\n}',start)+2
 helpers+=source[start:end]+'\n'
Path(sys.argv[2]).write_text('''#include <stdbool.h>
#include <assert.h>
#define MENUTAB_ROMBROWSE 1
#define MENUTAB_NORBROWSE 2
#define SUPPORT_NORGAMES
static bool launcher;
static bool browser_fast_reuse,browser_fast_valid[2];
static int browser_fast_selection[2];
static unsigned browser_fast_view[2],browser_fast_mode[2];
static int browser_motion_offset;
static unsigned framen,browser_view=1,artwork_mode=1;
static struct {unsigned menu_tab;struct {int selector;} browser,fbrowser;} smenu={.menu_tab=1};
static struct {struct {bool message;} qpop;struct {bool callback;} rtcpop;bool pop_num,alert_msg;} spop;
'''+helpers+'''static bool begin(void) {
'''+source[a:b]+'''return fast_carousel;}
static void finish(bool fast_carousel) {
'''+source[c:d]+'''}
int main(void) {
 assert(begin() && !browser_fast_reuse);finish(true);
 assert(begin() && browser_fast_reuse);
 framen=1;assert(begin() && !browser_fast_reuse);finish(true);
 framen=0;assert(begin() && browser_fast_reuse);
 smenu.browser.selector=1;assert(begin() && !browser_fast_reuse);finish(true);
 framen=1;assert(begin() && !browser_fast_reuse);finish(true);
 browser_view=2;assert(begin() && !browser_fast_reuse);finish(true);
 artwork_mode=2;assert(begin() && !browser_fast_reuse);finish(true);
 assert(begin());finish(true);assert(browser_fast_reuse || begin());
 smenu.menu_tab=MENUTAB_NORBROWSE;smenu.fbrowser.selector=smenu.browser.selector;
 assert(begin() && !browser_fast_reuse);finish(true);assert(begin() && browser_fast_reuse);
 smenu.fbrowser.selector++;assert(begin() && !browser_fast_reuse);finish(true);
 smenu.menu_tab=MENUTAB_ROMBROWSE;assert(begin() && !browser_fast_reuse);finish(true);
 for(unsigned kind=0;kind<6;++kind) {
  browser_fast_valid[0]=browser_fast_valid[1]=true;
  if(kind==0)spop.qpop.message=true;
  if(kind==1)spop.rtcpop.callback=true;
  if(kind==2)spop.pop_num=true;
  if(kind==3)spop.alert_msg=true;
  if(kind==4)smenu.menu_tab=7;
  if(kind==5){browser_motion_offset=48;}
  assert(!begin() && !browser_fast_reuse);
  assert(!browser_fast_valid[0] && !browser_fast_valid[1]);
  spop.qpop.message=spop.rtcpop.callback=spop.pop_num=spop.alert_msg=false;
  smenu.menu_tab=1;browser_motion_offset=0;
  assert(begin() && !browser_fast_reuse);
 }
 return 0;}
''')
PY
gcc -O2 -Wall -Wextra -Werror "$tmp/test.c" -o "$tmp/test"
"$tmp/test"
printf 'Production fast-page identity, page flips and modal invalidation passed.\n'
