#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$root/src/ingame_menu.c" "$tmp/test.c" <<'PY'
from pathlib import Path
import sys
source=Path(sys.argv[1]).read_text()
a=source.index('static void draw_menu_selection(');b=source.index('\n}',a)+2
Path(sys.argv[2]).write_text('''#include <stdint.h>
#include <assert.h>
#include <string.h>
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160
#define BL_COLOR 20
#define dup8(x) ((x)|((x)<<8))
static struct {const char *msg;} popup;
'''+source[a:b]+'''
int main(void) {
 _Alignas(4) uint8_t memory[240*160+32];
 for(unsigned blocked=0;blocked<2;++blocked) for(unsigned y=0;y<=160;++y) {
  memset(memory,0xa5,sizeof(memory));
  popup.msg=blocked?"confirm":0;
  draw_menu_selection(memory+16,y);
  for(unsigned row=0;row<160;++row) for(unsigned x=0;x<240;++x) {
   unsigned expected=!blocked && y<=144 && row>=y && row<y+16 && x>=4 && x<236?20:0xa5;
   assert(memory[16+row*240+x]==expected);
  }
  for(unsigned x=0;x<16;++x) assert(memory[x]==0xa5 && memory[240*160+16+x]==0xa5);
 }
 return 0;
}
''')
PY
gcc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie "$tmp/test.c" -o "$tmp/test"
"$tmp/test"
printf 'Production in-game selection bounds, pixels, popup suppression and canaries passed.\n'
