#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$root/src/menu.c" "$tmp/topbar.h" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text();a=s.index('static unsigned topbar_index(');b=s.index('\nvoid reload_theme(',a)
Path(sys.argv[2]).write_text(s[a:b])
PY
cat > "$tmp/test.c" <<'C'
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#define TOP_BG 246
#define FG_COLOR 16
#define THEME_DSSMM_LIGHT 6
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define dup8(a) ((a)|((a)<<8))
static unsigned menu_theme;
static void dma_memset16(volatile void *dst,uint16_t value,unsigned count) {volatile uint16_t *p=dst;while(count--)*p++=value;}
#include "topbar.h"
int main(void) {
 for(unsigned color=0;color<32768;++color) {
  assert(topbar_shade(color,3)==color);
  for(unsigned shade=0;shade<8;++shade) {
   unsigned adjusted=topbar_shade(color,shade);assert(adjusted<32768);
   if(shade)for(unsigned shift=0;shift<15;shift+=5)
    assert(((adjusted>>shift)&31)<=((topbar_shade(color,shade-1)>>shift)&31));
  }
 }
 uint16_t image[19208];
 for(unsigned theme=0;theme<8;++theme) {
  for(unsigned i=0;i<19208;++i)image[i]=0xeeee;
  menu_theme=theme;render_topbar_background((volatile uint8_t*)(image+4));
  for(unsigned i=0;i<4;++i)assert(image[i]==0xeeee && image[19204+i]==0xeeee);
  uint8_t *pixels=(uint8_t*)(image+4);
  for(unsigned i=16*240;i<240*160;++i)assert(pixels[i]==0xee);
  for(unsigned i=0;i<16*240;++i) {
   if(theme<6)assert(pixels[i]==FG_COLOR);
   else assert(pixels[i]==TOP_BG || (pixels[i]>=249 && pixels[i]<=254));
  }
  if(theme>=6) {
   assert(pixels[0]!=pixels[15*240]);
   assert(pixels[240]!=pixels[241]);
  }
 }
}
C
gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie -I "$tmp" "$tmp/test.c" -o "$tmp/test"
"$tmp/test"
printf 'Topbar shading preserves base colors, clamps channels, protects text/metadata slots and framebuffer bounds.\n'
