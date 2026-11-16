#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$root/src/menu.c" "$tmp/test.c" <<'PY'
from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
a=s.index('static void launcher_corner_span(');b=s.index('\nstatic void render_start(',a)
Path(sys.argv[2]).write_text('''#include <stdint.h>
#include <assert.h>
#define HI_COLOR 19
#define dup8(x) ((x)|((x)<<8))
#define MIN(a,b) ((a)<(b)?(a):(b))
'''+s[a:b]+'''
static uint16_t image[19216];
int main(void) {
 const unsigned boxes[][4]={{112,143,18,14},{176,0,64,16},{24,42,192,48},{111,143,19,14}};
 for(unsigned box=0;box<4;++box) {
  for(unsigned i=0;i<19216;++i)image[i]=0xeeee;
  uint8_t *frame=(uint8_t *)&image[8];
  unsigned x=boxes[box][0],y=boxes[box][1],w=boxes[box][2],h=boxes[box][3];
  launcher_corners(frame,x,y,w,h);
  assert(frame[y*240+x]==19 && frame[y*240+x+w-1]==19);
  assert(frame[(y+h-1)*240+x]==19 && frame[(y+h-1)*240+x+w-1]==19);
  assert(frame[(y+h/2)*240+x]==0xee && frame[(y+h/2)*240+x+w-1]==0xee);
  assert(frame[y*240+x+w/2]==0xee && frame[(y+h-1)*240+x+w/2]==0xee);
  for(unsigned yy=0;yy<160;++yy)for(unsigned xx=0;xx<240;++xx)
   if(xx<x || xx>=x+w || yy<y || yy>=y+h)assert(frame[yy*240+xx]==0xee);
  for(unsigned i=0;i<8;++i)assert(image[i]==0xeeee && image[19208+i]==0xeeee);
 }
 return 0;
}
''')
PY
gcc -O2 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-pie -no-pie "$tmp/test.c" -o "$tmp/test"
"$tmp/test"
printf 'Production Start markers retain four separate corners, aligned small boxes and masked Mode4 edges.\n'
