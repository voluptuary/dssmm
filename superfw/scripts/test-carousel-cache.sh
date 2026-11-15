#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
python3 - "$root/src/menu.c" "${CAROUSEL_TEST_SOURCE:-$tmp/test.c}" <<'PY'
from pathlib import Path
import sys
import os
source=Path(os.environ.get("MENU_SOURCE",sys.argv[1])).read_text()
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
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define AM_DIR 16
#define BG_COLOR 17
#define FG_COLOR 16
#define FT_COLOR 18
#define HI_COLOR 19
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
 CAROUSEL_STORAGE
 uint8_t artwork_bmp_buffer[ARTWORK_BMP_MAX_SIZE];
 t_centry *filefilter[5];
}
#ifdef TEST_CART_BUS
*sdr_state=(void *)0x08000000;
#define state (*sdr_state)
#else
state, *sdr_state=&state;
#endif
CAROUSEL_METADATA
static uint16_t carousel_hot_pixels[ARTWORK_MAX_PIXELS/2];
static bool carousel_hot_valid;
static int carousel_hot_index;
static unsigned hot_copies;
#define dma_memcpy16(d,s,n) (memcpy((d),(s),(n)*2),++hot_copies)
static bool artwork_cached;
static bool browser_fast_reuse, browser_fast_valid[2];
static unsigned artwork_mode=1;
static unsigned carousel_animation=1;
static int browser_motion_offset;
static unsigned loads, blits, icons, scales;
static unsigned browser_entry_icon(const t_centry *entry) { (void)entry; return 0; }
static void render_icon(int x,int y,unsigned icon) { (void)x;(void)y;(void)icon;++icons; }
static bool artwork_load(const char *rom,unsigned mode,uint8_t *pixels,size_t capacity,
 uint8_t *bmp,size_t size,artwork_info *info) {
 (void)mode;(void)pixels;(void)capacity;(void)bmp;(void)size;
 ++loads; *info=(artwork_info){mode ? 80 : 120,80,false}; return !strstr(rom,"missing");
}
bool artwork_blit_frame(volatile uint16_t *frame,const uint16_t *pixels,
 const artwork_info *info,unsigned x,unsigned y) {
 (void)frame;(void)pixels;
 assert(!(x&1) && x+info->width<=240 && y+info->height<=160);
 ++blits;return true;
}
bool artwork_blit_cropped(volatile uint16_t *frame,const uint16_t *pixels,
 const artwork_info *info,int x,int y) {
 (void)frame;(void)pixels;(void)info;(void)x;(void)y;++blits;return true;
}
bool artwork_blit_scaled(volatile uint16_t *frame,const uint16_t *pixels,
 const artwork_info *info,unsigned x,unsigned y,unsigned w,unsigned h) {
 (void)frame;(void)pixels;(void)info;assert(x+w<=240 && y+h<=160);++scales;++blits;return true;
}
bool artwork_blit_scaled_cropped(volatile uint16_t *frame,const uint16_t *pixels,
 const artwork_info *info,int x,int y,unsigned w,unsigned h) {
 (void)frame;(void)pixels;(void)info;(void)x;(void)y;(void)w;(void)h;++scales;++blits;return true;
}

'''
tests=r'''
int main(void) {
 static uint8_t frame[240*160];
 t_centry entries[5]={{0,"one.gba"},{0,"two.gba"},{0,"missing.gba"},{0,"four.gba"},{0,"five.gba"}};
 for(unsigned i=0;i<5;++i) state.filefilter[i]=&entries[i];
 strcpy(smenu.browser.cpath,"/"); smenu.browser.selector=1;smenu.browser.dispentries=3;
 render_browser_carousel(frame,false);assert(loads==1);
 render_browser_carousel(frame,false);assert(loads==2);
 render_browser_carousel(frame,false);assert(loads==3);
 render_browser_carousel(frame,false);assert(loads==3); /* negative cache too */
 unsigned stable_copies=hot_copies;
 render_browser_carousel(frame,false);
 assert(hot_copies==stable_copies); /* Stable selected pixels stay in GBA RAM. */
 unsigned prior_blits=blits;
 browser_motion_offset=128;smenu.browser.selector=2;
 render_browser_carousel(frame,true);assert(loads==3 && blits>prior_blits);
 assert(carousel_hot_valid && carousel_hot_index==1); /* Visual focus during motion, not the new logical selection. */
 artwork_mode=2;render_browser_carousel(frame,false);assert(loads==3);
 browser_motion_offset=0;render_browser_carousel(frame,false);assert(loads==4);
 state.carousel_art[0].pixels[0]=0x1234;
 browser_fast_valid[0]=browser_fast_valid[1]=true;
 browser_invalidate_artwork();
 assert(!browser_fast_valid[0] && !browser_fast_valid[1]);
 assert(!carousel_hot_valid);
 for(unsigned i=0;i<ARTWORK_CACHE_SLOTS;++i) assert(!CACHE_META[i].cached && !CACHE_META[i].valid);
 assert(state.carousel_art[0].pixels[0]==0x1234);
 render_browser_carousel(frame,false);assert(loads==5);
 browser_motion_offset=0;
 render_browser_carousel(frame,true);render_browser_carousel(frame,true);
 unsigned reuse_blits=blits;browser_fast_reuse=true;
 render_browser_carousel(frame,true);assert(blits==reuse_blits);
 browser_fast_reuse=false;
 /* A directory change must never display cache entries from the old path. */
 unsigned before_loads=loads, before_blits=blits, before_icons=icons;
 strcpy(smenu.browser.cpath,"/another/"); browser_motion_offset=128;
 render_browser_carousel(frame,false);
 assert(loads==before_loads && blits==before_blits && icons>before_icons);
 browser_motion_offset=0;render_browser_carousel(frame,false);
 assert(loads==before_loads+1);
 /* Five-slot lookahead survives a forward step with all near art ready. */
 browser_motion_offset=0;smenu.browser.dispentries=5;smenu.browser.selector=2;
 browser_invalidate_artwork();unsigned five_loads=loads;
 for(unsigned i=0;i<5;++i)render_browser_carousel(frame,false);
 assert(loads==five_loads+5);
 smenu.browser.selector=3;render_browser_carousel(frame,false);
 assert(loads==five_loads+5);
 /* Directories cannot trigger image loads or consume cached file artwork. */
 browser_invalidate_artwork();
 for(unsigned i=0;i<5;++i) entries[i].attr=AM_DIR;
 before_loads=loads;before_blits=blits;before_icons=icons;
 render_browser_carousel(frame,false);
 assert(loads==before_loads && blits==before_blits && icons>before_icons);
 for(unsigned i=0;i<5;++i)entries[i].attr=0;
 smenu.browser.selector=1;browser_invalidate_artwork();
 for(unsigned i=0;i<3;++i)render_browser_carousel(frame,false);
 browser_fast_valid[0]=browser_fast_valid[1]=true;browser_fast_reuse=true;
 unsigned far_blits=blits,far_copies=hot_copies,far_loads=loads;
 render_browser_carousel(frame,false);
 assert(loads==far_loads+1 && blits==far_blits && hot_copies==far_copies);
 assert(browser_fast_valid[0] && browser_fast_valid[1] && browser_fast_reuse);
 return 0;
}
'''
# Use the actual storage and metadata declarations, so a regression putting
# byte fields back on the cartridge bus is exercised by the ARM bus test too.
a=source.index('  struct {',source.index('  t_reg_entry_max nordata;'))
b=source.index('];',source.index('  } carousel_art',a))+2
preamble=preamble.replace('CAROUSEL_STORAGE',source[a:b])
if '} carousel_meta[' in source:
 b=source.index('];',source.index('} carousel_meta['))+2
 a=source.rfind('static struct {',0,b)
 preamble=preamble.replace('CAROUSEL_METADATA',source[a:b])
 tests=tests.replace('CACHE_META','carousel_meta')
else:
 preamble=preamble.replace('CAROUSEL_METADATA','')
 tests=tests.replace('CACHE_META','state.carousel_art')
if 'static void render_artwork_placeholder(' in source:
 preamble+='\n#define render_artwork_placeholder(...) ((void)0)\n'
Path(sys.argv[2]).write_text(preamble+function('browser_invalidate_artwork')+'\n'+function('render_browser_carousel')+'\n'+tests)
PY
if [[ -n "${CAROUSEL_TEST_SOURCE:-}" ]]; then exit 0; fi
gcc -O2 -Wall -Wextra -Wno-unused-variable -I "$root/src" "$tmp/test.c" "$root/src/browser_carousel.c" -o "$tmp/test"
"$tmp/test"
printf 'Production carousel cache loading, misses, motion and invalidation passed.\n'
