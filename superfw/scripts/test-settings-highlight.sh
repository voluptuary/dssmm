#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$root/src/menu.c" "$tmp/test.c" <<'PY'
from pathlib import Path
import sys
source=Path(sys.argv[1]).read_text();a=source.index('static void draw_menu_selection(');b=source.index('\n}',a)+2
Path(sys.argv[2]).write_text('''#include <stdint.h>
#include <assert.h>
#define SCREEN_WIDTH 240
#define FONT_STYLE_DSSMM 1
#define HI_COLOR 19
#define THEME_DSSMM_LIGHT 6
static unsigned menu_theme=6;
#define dup8(x) ((x)|((x)<<8))
static unsigned menu_font;
static void dma_memset16(volatile void *memory,uint16_t value,unsigned count) {
 volatile uint16_t *p=memory;while(count--)*p++=value;
}
'''+source[a:b]+'''
static uint16_t memory[19216];
int main(void) {
 for(menu_font=0;menu_font<2;++menu_font) {
  for(unsigned i=0;i<19216;++i)memory[i]=0xeeee;
  volatile uint8_t *frame=(volatile uint8_t*)&memory[8];
  unsigned y=menu_font?136:132,height=menu_font?13:16;
  draw_menu_selection(frame,y,112,224);
  for(unsigned row=0;row<160;++row)for(unsigned x=0;x<240;++x)
   assert(frame[row*240+x]==((row>=y && row<y+height && row<149 && x>=112 && x<224)?19:0xee));
  /* The SET bottom frame at151 and its gap at150 stay untouched. */
  for(unsigned x=0;x<240;++x)assert(frame[149*240+x]==0xee && frame[150*240+x]==0xee && frame[151*240+x]==0xee);
  for(unsigned i=0;i<8;++i)assert(memory[i]==0xeeee && memory[19208+i]==0xeeee);
 }
}
''')
PY
gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie "$tmp/test.c" -o "$tmp/test"
"$tmp/test"
printf 'Settings highlights fit native/DSSMM rows and preserve the bottom frame/gap.\n'
