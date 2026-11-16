/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "dssmm_background.h"
#include "dssmm_background_data.h"
#ifdef DSSMM_EXTERNAL_BACKGROUNDS
#include "dssmm_background_offsets.h"
extern void *font_base_addr;
#endif
void dssmm_background_palette(volatile uint16_t *palette,bool dark) {
  const uint16_t *source=dark?dssmm_dark_palette:dssmm_light_palette;
  for(unsigned i=0;i<16;++i)palette[i]=source[i];
}
/* Native DE SD_LIST is alternating solid scanlines; no extra framebuffer.
 * Coordinates must be even in Mode4. Keep all VRAM stores halfword-wide. */
void dssmm_background_clear(volatile uint8_t *frame,bool dark,unsigned x,unsigned y,unsigned width,unsigned height) {
  if(x>=240 || y>=160 || (x&1) || (width&1))return;
  if(width>240-x)width=240-x;
  if(height>160-y)height=160-y;
  const uint16_t *palette=dark?dssmm_dark_palette:dssmm_light_palette;
  unsigned a=0,b=0;
  /* Exact SD_LIST values: light28/31; dark3/0. */
  for(unsigned i=0;i<16;++i) {
    if(palette[i]==(dark?0x0c63:0x739c))a=i;
    if(palette[i]==(dark?0x0000:0x7fff))b=i;
  }
  for(unsigned row=y;row<y+height;++row) {
    unsigned color=(row&1)?b:a;
    volatile uint16_t *dest=(volatile uint16_t *)(frame+row*240+x);
    for(unsigned col=0;col<width/2;++col)dest[col]=color|(color<<8);
  }
}
void dssmm_background_draw(volatile uint8_t *frame,bool dark,unsigned kind) {
  if(kind==DSSMM_BG_GRID || kind>DSSMM_BG_SETTINGS){dssmm_background_clear(frame,dark,0,0,240,160);return;}
#ifdef DSSMM_EXTERNAL_BACKGROUNDS
#ifdef FONTS_EXT
  const unsigned base=DSSMM_BG_BASE_EXTENDED;
#else
  const unsigned base=DSSMM_BG_BASE_STANDARD;
#endif
  const uint8_t *asset=(const uint8_t *)font_base_addr+base;
  if(*(const uint32_t *)asset!=DSSMM_BG_MAGIC) {
    dssmm_background_clear(frame,dark,0,0,240,160);return;
  }
  const uint16_t *source=(const uint16_t *)(asset+dssmm_stream_offsets[dark][kind-1]);
  unsigned words=dssmm_stream_lengths[dark][kind-1];
#else
  const uint16_t *source=kind==DSSMM_BG_SETTINGS?(dark?dssmm_dark_settings:dssmm_light_settings):(dark?dssmm_dark_start:dssmm_light_start);
  unsigned words=kind==DSSMM_BG_SETTINGS?(dark?sizeof(dssmm_dark_settings):sizeof(dssmm_light_settings)):(dark?sizeof(dssmm_dark_start):sizeof(dssmm_light_start));
  words/=2;
#endif
  volatile uint16_t *out=(volatile uint16_t *)frame;
  unsigned pos=0;
  while(pos<19200) {
    if(!words--){dssmm_background_clear(frame,dark,0,0,240,160);return;}
    unsigned token=*source++;
    if(token<256)out[pos++]=(token&15)|((token>>4)<<8);
    else {
      unsigned length=(token>>11)+3,dist=(token&2047)+1;
      if(token<2048 || dist>pos || length>19200-pos){dssmm_background_clear(frame,dark,0,0,240,160);return;}
      while(length--){out[pos]=out[pos-dist];++pos;}
    }
  }
}
