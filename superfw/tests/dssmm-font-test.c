/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "fonts/font_render.h"
static const struct {
  char magic[2];uint8_t version,count;uint32_t size;
  uint32_t first,last,flags,offset;uint16_t columns[16];
} fallback={{'F','O'},1,1,56,0x3a9,0x3a9,1,0,{1,2,4,8}};
void *font_base_addr=(void *)&fallback;
static uint16_t frame[240*160/2+2],reference[240*160/2+2];
static uint8_t rows[95*12];
int main(int argc,char **argv) {
 assert(argc==2);FILE *f=fopen(argv[1],"rb");assert(f);
 assert(fread(rows,1,sizeof(rows),f)==sizeof(rows));assert(fgetc(f)==EOF);fclose(f);
 font_set_style(FONT_STYLE_NATIVE);unsigned native_width=font_width("Game Boy Advance");
 memset(reference,17,sizeof(reference));
 draw_text_idx8_bus16("Game Boy Advance",(uint8_t *)(reference+1)+20*240+8,240,37);
 unsigned arrow_width=font_width("▸");assert(font_width("Ω")==16);
 font_set_style(FONT_STYLE_DSSMM);assert(font_get_style()==FONT_STYLE_DSSMM);
 for(unsigned c=32;c<=126;++c)for(unsigned origin=8;origin<=9;++origin) {
  char text[2]={(char)c,0};unsigned width=c=='~'?7:6;
  assert(font_width(text)==width);
  memset(frame,17,sizeof(frame));
  draw_text_idx8_bus16(text,(uint8_t *)(frame+1)+20*240+origin,240,37);
  for(unsigned y=0;y<160;++y)for(unsigned x=0;x<240;++x) {
   unsigned expected=17;
   if(y>=22 && y<34 && x>=origin && x<origin+width &&
      (rows[(c-32)*12+y-22]&(0x80>>(x-origin))))expected=37;
   assert(((uint8_t *)(frame+1))[y*240+x]==expected);
  }
  assert(frame[0]==0x1111 && frame[240*160/2+1]==0x1111);
 }
 assert(font_width("Game Boy Advance")==16*6);
 assert(font_width("▸")==arrow_width && font_width("Ω")==16);
 assert(font_width("AΩB")==28);
 assert(font_width_cap("AΩB",22)==3 && font_width_cap("AΩB",21)==1);
 assert(font_width_lcap("AΩB",22)==1);
 const char *text="Hello ~ world";unsigned full=font_width(text);
 static _Alignas(4) uint8_t complete[240*16];memset(complete,17,sizeof(complete));
 draw_text_idx8_bus16(text,complete,240,37);
 for(unsigned skip=0;skip<full+2;++skip)for(unsigned width=1;width<=full+2;++width) {
  memset(frame,17,sizeof(frame));
  draw_text_idx8_bus16_range(text,(uint8_t *)(frame+1)+8,skip,width,240,37);
  for(unsigned y=0;y<16;++y)for(unsigned x=0;x<240;++x) {
   unsigned expected=x>=8 && x<8+width && skip+x-8<full ? complete[y*240+skip+x-8]:17;
   assert(((uint8_t *)(frame+1))[y*240+x]==expected);
  }
  assert(frame[0]==0x1111 && frame[240*160/2+1]==0x1111);
 }
 font_set_style(99);assert(font_get_style()==FONT_STYLE_NATIVE);
 assert(font_width("Game Boy Advance")==native_width);
 memset(frame,17,sizeof(frame));
 draw_text_idx8_bus16("Game Boy Advance",(uint8_t *)(frame+1)+20*240+8,240,37);
 assert(!memcmp(frame,reference,sizeof(frame)));
 puts("DSSMM glyph pixels, metrics, Unicode fallback, scrolling and native restoration passed.");
 return 0;
}
