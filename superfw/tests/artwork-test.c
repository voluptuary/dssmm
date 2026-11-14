/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <string.h>
#include "artwork.h"
static uint8_t file[54 + 120 * 160];
_Alignas(2) static uint8_t output[ARTWORK_MAX_PIXELS + 4];
static unsigned fail_offset;
static unsigned reads;
static void put16(unsigned off, unsigned value) { file[off] = value; file[off+1] = value >> 8; }
static void put32(unsigned off, uint32_t value) { put16(off,value); put16(off+2,value>>16); }
static bool read_file(void *ctx, uint32_t off, void *out, size_t size) {
  (void)ctx;
  ++reads;
  if (off == fail_offset || off > sizeof(file) || size > sizeof(file)-off) return false;
  memcpy(out, file+off, size); return true;
}
static void fixture(unsigned width, bool marked) {
  memset(file, 0, sizeof(file)); memset(output, 0xa5, sizeof(output));
  memcpy(file,"BM",2); put32(2,54+width*160); put32(10,54); put32(14,40);
  put32(18,width); put32(22,(uint32_t)-80); put16(26,1); put16(28,16);
  put32(34,width*160); if (marked) memcpy(file+6,"DST1",4);
  put16(56,31); put16(58,31<<5); put16(60,31<<10);
  fail_offset = UINT32_MAX;
}
static bool decode(unsigned width, size_t capacity, artwork_info *info) {
  return artwork_decode(read_file,0,54+width*160,output+2,capacity,info);
}
int main(void) {
  artwork_info info;
  for (unsigned width=80; width<=120; width+=40) {
    for (unsigned marked=0; marked<2; ++marked) {
      fixture(width,marked); assert(decode(width,width*80,&info));
      assert(info.width==width && info.height==80 && info.transparent==marked);
      assert(output[0]==0xa5 && output[width*80+2]==0xa5);
      assert(output[2]==(marked ? 0 : ARTWORK_PALETTE_BASE));
      assert(output[3]==25 && output[4]==50 && output[5]==200);
    }
    fixture(width,false); assert(!decode(width,width*80-1,&info));
    fixture(width,false); fail_offset=54+width*2*78;
    assert(!decode(width,ARTWORK_MAX_PIXELS,&info));
  }
  const unsigned fields[]={0,10,14,18,22,26,28,30};
  for (unsigned i=0;i<sizeof(fields)/sizeof(fields[0]);++i) {
    fixture(120,false); file[fields[i]] ^= 1;
    assert(!decode(120,ARTWORK_MAX_PIXELS,&info));
  }
  fixture(120,false); put32(10,UINT32_MAX); assert(!decode(120,ARTWORK_MAX_PIXELS,&info));
  fixture(120,false); put32(2,0); put32(34,0); reads=0;
  assert(decode(120,ARTWORK_MAX_PIXELS,&info) && reads==41);
  fixture(120,false); put32(22,80); put16(54,31); put16(54+79*240,31<<10);
  assert(decode(120,ARTWORK_MAX_PIXELS,&info));
  assert(output[2]==ARTWORK_PALETTE_BASE+180 && output[2+(79*120)]==ARTWORK_PALETTE_BASE+5);
  fixture(120,false); put16(54,0x8000); put16(56,0x801f);
  assert(decode(120,ARTWORK_MAX_PIXELS,&info));
  assert(output[2]==ARTWORK_PALETTE_BASE && output[3]==ARTWORK_PALETTE_BASE+5);
  fixture(120,true); put16(54,0x8000);
  assert(decode(120,ARTWORK_MAX_PIXELS,&info) && output[2]==0);
  fixture(120,false); assert(!artwork_decode(read_file,0,53,output,sizeof(output),&info));
  assert(!artwork_decode(0,0,sizeof(file),output,sizeof(output),&info));
  static uint16_t frame[240*160/2 + 2];
  for (unsigned i=0; i<sizeof(frame)/sizeof(frame[0]); ++i) frame[i]=0x1111;
  fixture(120,true); assert(decode(120,ARTWORK_MAX_PIXELS,&info));
  assert(artwork_blit_frame(frame+1,(uint16_t *)(output+2),&info,120,40));
  unsigned pos=1+40*120+60;
  assert(frame[pos]==(25*256+17));  // transparent low pixel keeps background
  assert(frame[pos+1]==(200*256+50));
  assert(frame[0]==0x1111 && frame[sizeof(frame)/sizeof(frame[0])-1]==0x1111);
  assert(frame[pos-1]==0x1111 && frame[pos+60]==0x1111);
  assert(!artwork_blit_frame(frame,(uint16_t *)(output+2),&info,121,40));
  assert(!artwork_blit_frame(frame,(uint16_t *)(output+2),&info,120,81));
  assert(artwork_blit_scaled(frame+1,(uint16_t *)(output+2),&info,10,20,84,56));
  assert(frame[1+20*120+5] == 0x1911); /* marked source pixel preserves destination */
  assert(frame[0]==0x1111 && frame[sizeof(frame)/sizeof(frame[0])-1]==0x1111);
  assert(!artwork_blit_scaled(frame,(uint16_t *)(output+2),&info,10,20,85,56));
  assert(!artwork_blit_scaled(frame,(uint16_t *)(output+2),&info,10,20,84,141));
  fixture(120,false); assert(decode(120,ARTWORK_MAX_PIXELS,&info));
  assert(artwork_blit_frame(frame,(uint16_t *)(output+2),&info,20,30));
  assert(frame[30*120+10] == (ARTWORK_PALETTE_BASE | ((ARTWORK_PALETTE_BASE + 5) << 8)));
  assert(!artwork_decode(read_file,0,sizeof(file),output+1,sizeof(output)-1,&info));
  assert(!artwork_blit_scaled(frame,(uint16_t *)(output+2),&info,0,0,242,80));
  assert(!artwork_blit_scaled(frame,(uint16_t *)(output+2),&info,0,0,120,161));
  assert(!artwork_blit_scaled(frame,(uint16_t *)(output+2),&info,0,0,UINT32_MAX-1,80));
  /* Compare optimized scaling with the original integer sampling equation. */
  const unsigned widths[]={2,40,48,56,60,80,84,120,240};
  const unsigned heights[]={1,32,40,56,80,160};
  for(unsigned marked=0;marked<2;++marked) {
    fixture(120,marked);assert(decode(120,ARTWORK_MAX_PIXELS,&info));
    memset(frame,17,sizeof(frame));
    assert(artwork_blit_frame(frame+1,(uint16_t *)(output+2),&info,0,0));
    for(unsigned y=0;y<80;++y) for(unsigned x=0;x<120;++x) {
      uint8_t expected=output[2+y*120+x];
      if(!expected) expected=17;
      assert(((uint8_t *)(frame+1))[y*240+x]==expected);
    }
    for(unsigned wi=0;wi<sizeof(widths)/sizeof(widths[0]);++wi)
      for(unsigned hi=0;hi<sizeof(heights)/sizeof(heights[0]);++hi) {
        unsigned w=widths[wi],h=heights[hi];
        memset(frame,17,sizeof(frame));
        assert(artwork_blit_scaled(frame+1,(uint16_t *)(output+2),&info,0,0,w,h));
        for(unsigned y=0;y<h;++y) for(unsigned x=0;x<w;++x) {
          uint8_t expected=output[2+(y*80/h)*120+x*120/w];
          if(!expected) expected=17;
          assert(((uint8_t *)(frame+1))[y*240+x]==expected);
        }
        assert(frame[0]==0x1111 && frame[240*160/2+1]==0x1111);
      }
  }
  assert(artwork_palette_color(0)==0 && artwork_palette_color(215)==0x7fff);
  assert(artwork_palette_color(5)==31 && artwork_palette_color(30)==(31<<5));
  assert(artwork_palette_color(180)==(31<<10));
  return 0;
}
