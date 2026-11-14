#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$root/src/menu.c" "$tmp/test.c" <<'PY'
from pathlib import Path
import sys
source=Path(sys.argv[1]).read_text()
def function(name):
 start=source.index('static void '+name+'(')
 end=source.index('\n}',start)+2
 return source[start:end]
preamble=r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "artwork.h"
#include "browser_carousel.h"
#define MAX_FN_LEN 256
#define AM_DIR 16
#define BG_COLOR 17
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 160
#define MSG_BROW_EMPTY 0
#define dma_memset16(...) ((void)0)
#define dup8(x) (x)
#define draw_central_text(...) ((void)0)
#define draw_central_text_ovf(...) ((void)0)
#define draw_text_leftovf(...) ((void)0)
#define draw_text_ovf_rotate(...) ((void)0)
#define draw_rightj_text(...) ((void)0)
#define npf_snprintf snprintf
#define strbcpy(d,s,n) snprintf(d,n,"%s",s)
typedef struct { unsigned attr; char fname[32]; } t_centry;
static struct { struct { int selector,dispentries; char cpath[64]; } browser; } smenu;
static struct state {
 struct { bool cached,valid; unsigned mode; char rom[MAX_FN_LEN]; artwork_info info;
 uint16_t pixels[ARTWORK_MAX_PIXELS/2]; } carousel_art[3];
 uint8_t artwork_bmp_buffer[ARTWORK_BMP_MAX_SIZE];
 t_centry *filefilter[3];
} state, *sdr_state=&state;
static bool artwork_cached;
static unsigned artwork_mode=1;
static int browser_motion_offset;
static unsigned loads, blits, icons;
static unsigned browser_entry_icon(const t_centry *entry) { (void)entry; return 0; }
static void render_icon(int x,int y,unsigned icon) { (void)x;(void)y;(void)icon;++icons; }
static bool artwork_load(const char *rom,unsigned mode,uint8_t *pixels,size_t capacity,
 uint8_t *bmp,size_t size,artwork_info *info) {
 (void)mode;(void)pixels;(void)capacity;(void)bmp;(void)size;
 ++loads; *info=(artwork_info){120,80,false}; return !strstr(rom,"missing");
}
bool artwork_blit_frame(volatile uint16_t *frame,const uint16_t *pixels,
 const artwork_info *info,unsigned x,unsigned y) {
 (void)frame;(void)pixels;(void)info;(void)x;(void)y;++blits;return true;
}
bool artwork_blit_scaled(volatile uint16_t *frame,const uint16_t *pixels,
 const artwork_info *info,unsigned x,unsigned y,unsigned w,unsigned h) {
 (void)w;(void)h;return artwork_blit_frame(frame,pixels,info,x,y);
}
'''
tests=r'''
int main(void) {
 static uint8_t frame[240*160];
 t_centry entries[3]={{0,"one.gba"},{0,"two.gba"},{0,"missing.gba"}};
 for(unsigned i=0;i<3;++i) state.filefilter[i]=&entries[i];
 strcpy(smenu.browser.cpath,"/"); smenu.browser.selector=1;smenu.browser.dispentries=3;
 render_browser_carousel(frame,false);assert(loads==1);
 render_browser_carousel(frame,false);assert(loads==2);
 render_browser_carousel(frame,false);assert(loads==3);
 render_browser_carousel(frame,false);assert(loads==3); /* negative cache too */
 unsigned prior_blits=blits;
 browser_motion_offset=128;smenu.browser.selector=2;
 render_browser_carousel(frame,true);assert(loads==3 && blits>prior_blits);
 artwork_mode=2;render_browser_carousel(frame,false);assert(loads==3);
 browser_motion_offset=0;render_browser_carousel(frame,false);assert(loads==4);
 state.carousel_art[0].pixels[0]=0x1234;
 browser_invalidate_artwork();
 for(unsigned i=0;i<3;++i) assert(!state.carousel_art[i].cached && !state.carousel_art[i].valid);
 assert(state.carousel_art[0].pixels[0]==0x1234);
 render_browser_carousel(frame,false);assert(loads==5);
 /* A directory change must never display cache entries from the old path. */
 unsigned before_loads=loads, before_blits=blits, before_icons=icons;
 strcpy(smenu.browser.cpath,"/another/"); browser_motion_offset=128;
 render_browser_carousel(frame,false);
 assert(loads==before_loads && blits==before_blits && icons>before_icons);
 browser_motion_offset=0;render_browser_carousel(frame,false);
 assert(loads==before_loads+1);
 /* Directories cannot trigger image loads or consume cached file artwork. */
 browser_invalidate_artwork();
 for(unsigned i=0;i<3;++i) entries[i].attr=AM_DIR;
 before_loads=loads;before_blits=blits;before_icons=icons;
 render_browser_carousel(frame,false);
 assert(loads==before_loads && blits==before_blits && icons>before_icons);
 return 0;
}
'''
Path(sys.argv[2]).write_text(preamble+function('browser_invalidate_artwork')+'\n'+function('render_browser_carousel')+'\n'+tests)
PY
gcc -O2 -Wall -Wextra -Wno-unused-variable -I "$root/src" "$tmp/test.c" "$root/src/browser_carousel.c" -o "$tmp/test"
"$tmp/test"
printf 'Production carousel cache loading, misses, motion and invalidation passed.\n'
