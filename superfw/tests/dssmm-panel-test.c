/* Compare production panel composition to the original Settings pixels. */
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include "dssmm_background.h"
#include "res/dssmm-panel.h"
#define THEME_DSSMM_DARK 7
#define dup8(x) ((x)|((x)<<8))
static unsigned menu_theme;
static void dma_memset16(volatile void *dst,uint16_t value,uint16_t count) {volatile uint16_t *p=dst;while(count--)*p++=value;}
#include "panel-under-test.h"
static uint16_t image[19216],original[38400];
int main(int argc,char **argv) {
 assert(argc==3);
 for(unsigned dark=0;dark<2;dark++) {
  FILE *f=fopen(argv[1+dark],"rb");assert(f);assert(fread(original,2,38400,f)==38400);fclose(f);
  uint16_t palette[16];dssmm_background_palette(palette,dark);
  for(unsigned i=0;i<19216;i++)image[i]=0xeeee;
  uint8_t *frame=(uint8_t*)&image[8];menu_theme=6+dark;draw_lined_panel(frame);
  for(unsigned y=0;y<16;y++)for(unsigned x=0;x<240;x++)assert(frame[y*240+x]==0xee);
  for(unsigned y=21;y<152;y++)for(unsigned x=2;x<238;x++) {
   unsigned sx=x<4?x+12:x<234?120:x-10;
   assert(palette[frame[y*240+x]]==((x==4 && y>=23 && y<=149)?0:original[y*240+sx]));
  }
  for(unsigned i=0;i<8;i++)assert(image[i]==0xeeee&&image[19208+i]==0xeeee);
 }
 puts("Light/Dark panels match original right-panel stripes/bevel; headers and framebuffer guards preserved.");
}
