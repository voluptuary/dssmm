/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <string.h>
#include "artwork.h"
static uint8_t file[54 + 120 * 160], output[ARTWORK_MAX_PIXELS + 2];
static unsigned fail_offset;
static void put16(unsigned off, unsigned value) { file[off] = value; file[off+1] = value >> 8; }
static void put32(unsigned off, uint32_t value) { put16(off,value); put16(off+2,value>>16); }
static bool read_file(void *ctx, uint32_t off, void *out, size_t size) {
  (void)ctx;
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
  return artwork_decode(read_file,0,54+width*160,output+1,capacity,info);
}
int main(void) {
  artwork_info info;
  for (unsigned width=80; width<=120; width+=40) {
    for (unsigned marked=0; marked<2; ++marked) {
      fixture(width,marked); assert(decode(width,width*80,&info));
      assert(info.width==width && info.height==80 && info.transparent==marked);
      assert(output[0]==0xa5 && output[width*80+1]==0xa5);
      assert(output[1]==(marked ? 0 : ARTWORK_PALETTE_BASE));
      assert(output[2]==25 && output[3]==50 && output[4]==200);
    }
    fixture(width,false); assert(!decode(width,width*80-1,&info));
    fixture(width,false); fail_offset=54+width*2*79;
    assert(!decode(width,ARTWORK_MAX_PIXELS,&info));
  }
  const unsigned fields[]={0,2,10,14,18,22,26,28,30,34};
  for (unsigned i=0;i<sizeof(fields)/sizeof(fields[0]);++i) {
    fixture(120,false); file[fields[i]] ^= 1;
    assert(!decode(120,ARTWORK_MAX_PIXELS,&info));
  }
  fixture(120,false); put32(10,UINT32_MAX); assert(!decode(120,ARTWORK_MAX_PIXELS,&info));
  fixture(120,false); put16(54,0x8000); assert(!decode(120,ARTWORK_MAX_PIXELS,&info));
  fixture(120,false); assert(!artwork_decode(read_file,0,53,output,sizeof(output),&info));
  assert(!artwork_decode(0,0,sizeof(file),output,sizeof(output),&info));
  assert(artwork_palette_color(0)==0 && artwork_palette_color(215)==0x7fff);
  assert(artwork_palette_color(5)==31 && artwork_palette_color(30)==(31<<5));
  assert(artwork_palette_color(180)==(31<<10));
  return 0;
}
