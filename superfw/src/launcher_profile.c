/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <string.h>
#include "launcher_profile.h"
extern volatile unsigned frame_count;
static uint32_t clock_seconds=UINT32_MAX,clock_frame;
void launcher_clock_set(uint32_t seconds) {
  clock_seconds=seconds==UINT32_MAX?seconds:seconds%86400;
  clock_frame=frame_count;
}
uint32_t launcher_clock_get(void) {
  if(clock_seconds==UINT32_MAX)return UINT32_MAX;
  /* GBA frames last280896 cycles at16777216Hz; no 60Hz clock drift. */
  uint32_t elapsed=(uint64_t)(uint32_t)(frame_count-clock_frame)*280896u>>24;
  return (clock_seconds+elapsed)%86400;
}
void launcher_name_parse(char out[LAUNCHER_NAME_BYTES],const char *data,size_t bytes) {
  size_t i=0,pos=0;unsigned chars=0;
  if(bytes>=3 && !memcmp(data,"\xef\xbb\xbf",3))i=3;
  while(i<bytes && chars<12) {
    unsigned c=(uint8_t)data[i],length=c<128?1:c>=0xc2 && c<=0xdf?2:c>=0xe0 && c<=0xef?3:c>=0xf0 && c<=0xf4?4:0;
    if(c<32 || c==127)break;
    /* Validate bounds before copying any continuation. */
    unsigned valid=length && length<=bytes-i;
    for(unsigned j=1;valid && j<length;++j)valid=((uint8_t)data[i+j]&0xc0)==0x80;
    if(valid && length>=3) {
      unsigned next=(uint8_t)data[i+1];
      if((c==0xe0 && next<0xa0) || (c==0xed && next>=0xa0) || (c==0xf0 && next<0x90) || (c==0xf4 && next>=0x90))valid=0;
    }
    if(!valid){out[pos++]='?';++i;}
    else {memcpy(out+pos,data+i,length);pos+=length;i+=length;}
    ++chars;
  }
  out[pos]=0;
}

unsigned launcher_title_top(unsigned rows,unsigned dssmm_font) {
  unsigned maximum=dssmm_font?3:2;
  if(!rows)rows=1;
  if(rows>maximum)rows=maximum;
  /* Native font conversion adds two blank rows above the DE glyph bitmap. */
  if(dssmm_font)return rows==1?58:rows==2?53:47;
  return 66-rows*8;
}
