#include "compiler.h"
/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "dssmm_background.h"
#include "dssmm_background_data.h"
#ifdef __GBA__
#include "gbahw.h"
#endif
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
IWRAM_CODE NOINLINE void dssmm_background_draw(volatile uint8_t *frame,bool dark,unsigned kind) {
  if(kind==DSSMM_BG_LIST || kind>DSSMM_BG_HELP){dssmm_background_clear(frame,dark,0,0,240,160);return;}
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
  static const uint16_t * const streams[2][6]={
    {dssmm_light_start,dssmm_light_settings,dssmm_light_horizontal,dssmm_light_vertical,dssmm_light_grid,dssmm_light_help},
    {dssmm_dark_start,dssmm_dark_settings,dssmm_dark_horizontal,dssmm_dark_vertical,dssmm_dark_grid,dssmm_dark_help}
  };
  static const unsigned lengths[2][6]={
    {sizeof(dssmm_light_start)/2,sizeof(dssmm_light_settings)/2,sizeof(dssmm_light_horizontal)/2,sizeof(dssmm_light_vertical)/2,sizeof(dssmm_light_grid)/2,sizeof(dssmm_light_help)/2},
    {sizeof(dssmm_dark_start)/2,sizeof(dssmm_dark_settings)/2,sizeof(dssmm_dark_horizontal)/2,sizeof(dssmm_dark_vertical)/2,sizeof(dssmm_dark_grid)/2,sizeof(dssmm_dark_help)/2}
  };
  const uint16_t *source=streams[dark][kind-1];
  unsigned words=lengths[dark][kind-1];
#endif
  volatile uint16_t *out=(volatile uint16_t *)frame;
  unsigned pos=0;
  while(pos<19200) {
    if(!words--){dssmm_background_clear(frame,dark,0,0,240,160);return;}
    unsigned token=*source++,length=(token>>8)+1;
    uint16_t value=(token&15)|((token&240)<<4);
    if(length>19200-pos){dssmm_background_clear(frame,dark,0,0,240,160);return;}
#ifdef __GBA__
    if(length>=8)dma_memset16(out+pos,value,length);
    else
#endif
      for(unsigned i=0;i<length;++i)out[pos+i]=value;
    pos+=length;
  }
}
