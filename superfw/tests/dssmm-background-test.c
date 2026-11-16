#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "dssmm_background.h"
static uint16_t buffer[19200+16],palette[16];
int main(int argc,char **argv) {
 assert(argc==13);
 for(unsigned dark=0;dark<2;++dark)for(unsigned kind=1;kind<=6;++kind) {
  for(unsigned i=0;i<19216;++i)buffer[i]=0x55aa;
  uint8_t *frame=(uint8_t *)&buffer[8];
  dssmm_background_palette(palette,dark);
  dssmm_background_draw(frame,dark,kind);
  FILE *f=fopen(argv[1+dark*6+kind-1],"rb");assert(f);
  for(unsigned i=0;i<38400;++i) {
    unsigned lo=fgetc(f),hi=fgetc(f);assert(lo<256 && hi<256);
    assert(frame[i]<16);assert(palette[frame[i]]==(lo|(hi<<8)));
  }
  assert(fgetc(f)==EOF);fclose(f);
  for(unsigned i=0;i<8;++i)assert(buffer[i]==0x55aa && buffer[19208+i]==0x55aa);
  dssmm_background_draw(frame,dark,false);
  for(unsigned y=0;y<160;++y)for(unsigned x=0;x<240;++x)
    assert(palette[frame[y*240+x]]==(dark?((y&1)?0:0x0c63):((y&1)?0x7fff:0x739c)));
  dssmm_background_clear(frame,dark,1,0,238,160); /* Odd Mode4 writes rejected. */
  dssmm_background_clear(frame,dark,238,158,240,160); /* Bounds clipped. */
  for(unsigned i=0;i<8;++i)assert(buffer[i]==0x55aa && buffer[19208+i]==0x55aa);
 }
 puts("DSSMM Start/SET/carousel/grid/Help backgrounds match every stored source pixel; stripes and guards pass.");
}
