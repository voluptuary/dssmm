#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie \
 -I "$root/src" "$root/tests/launcher-profile-test.c" "$root/src/launcher_profile.c" -o "$tmp/test"
"$tmp/test"
printf 'Username UTF8/first-line/12-character bounds and GBA-frame clock rollover/unset state passed.\n'
python3 - "$root/src/menu.c" "$tmp/editor.c" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
out='''#include <stdio.h>
#include <string.h>
#define npf_snprintf snprintf
static unsigned menu_clock_24=1;
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include "util.h"
#include "launcher_profile.h"
volatile unsigned frame_count;
#define KEY_BUTTA 1
#define KEY_BUTTB 2
#define KEY_BUTTRIGHT 16
#define KEY_BUTTLEFT 32
#define KEY_BUTTUP 64
#define KEY_BUTTDOWN 128
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MSG_ERR_SETSAVE 0
static unsigned writes;
static uint32_t menu_clock_seed,rtcvalue_default=12345;
static struct {const char *alert_msg;struct {t_dec_date val;int selector;bool menu_clock;void (*callback)(void);} rtcpop;} spop;
static void launcher_invalidate(void){}
static void settings_save_request(void){++writes;}
'''
for name in ('format_menu_clock','accept_menu_clock','open_menu_clock'):
 a=s.index('static void '+name+'(');b=s.index('\n}',a)+2;out+=s[a:b]+'\n'
a=s.index('    if (newkeys & KEY_BUTTLEFT)',s.index('  else if (spop.rtcpop.callback)',s.index('void menu_keypress(')))
b=s.index('  }\n  else if (spop.pop_num)',a)
out+='static void edit(unsigned newkeys) {\n'+s[a:b]+'''}
int main(void) {
 char text[12];
 format_menu_clock(text,sizeof(text),0);assert(!strcmp(text,"00:00:00"));
 menu_clock_24=0;
 format_menu_clock(text,sizeof(text),0);assert(!strcmp(text,"12:00 AM"));
 format_menu_clock(text,sizeof(text),43200);assert(!strcmp(text,"12:00 PM"));
 format_menu_clock(text,sizeof(text),86340);assert(!strcmp(text,"11:59 PM"));
 format_menu_clock(text,sizeof(text),UINT32_MAX);assert(!strcmp(text,"--:--:--"));
 launcher_clock_set(UINT32_MAX);open_menu_clock();assert(spop.rtcpop.val.hour==12 && spop.rtcpop.val.min==0);
 assert(spop.rtcpop.selector==3 && spop.rtcpop.menu_clock && spop.rtcpop.callback);
 edit(KEY_BUTTLEFT);assert(spop.rtcpop.selector==3);
 edit(KEY_BUTTDOWN);assert(spop.rtcpop.val.hour==11);
 edit(KEY_BUTTRIGHT);edit(KEY_BUTTRIGHT);assert(spop.rtcpop.selector==4);
 edit(KEY_BUTTDOWN);assert(spop.rtcpop.val.min==59);
 edit(KEY_BUTTB);assert(!spop.rtcpop.callback && !writes && launcher_clock_get()==UINT32_MAX);
 open_menu_clock();spop.rtcpop.val.hour=23;spop.rtcpop.val.min=59;edit(KEY_BUTTA);
 assert(!spop.rtcpop.callback && writes==1 && menu_clock_seed==86340 && launcher_clock_get()==86340);
 assert(rtcvalue_default==12345);
}
'''
Path(sys.argv[2]).write_text(out)
PY
gcc -O2 -Wall -Wextra -Werror -Wno-sign-compare -ffunction-sections -fdata-sections -Wl,--gc-sections \
 -fsanitize=address,undefined -fno-pie -no-pie -I "$root" -I "$root/src" \
 "$tmp/editor.c" "$root/src/launcher_profile.c" "$root/src/util.c" -o "$tmp/editor"
"$tmp/editor"
printf 'Production clock editor field bounds, cancel, accept, persistence and game-RTC isolation passed.\n'
