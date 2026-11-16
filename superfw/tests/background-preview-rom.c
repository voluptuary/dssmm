/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdint.h>
#include <stdbool.h>
#include "dssmm_background.h"
#define SCREEN ((volatile uint8_t*)0x06000000)
#define PALETTE ((volatile uint16_t*)0x05000000)
#define DISPCNT (*(volatile uint16_t*)0x04000000)
#define VCOUNT (*(volatile uint16_t*)0x04000006)
#define KEYS (*(volatile uint16_t*)0x04000130)
int main(void) {
  unsigned scene=0,old_keys=0;bool redraw=true;
  DISPCNT=4|0x0400;
  for(;;) {
    unsigned keys=(~KEYS)&0x3ff,fresh=keys&~old_keys;old_keys=keys;
    if(fresh&16){scene=(scene+1)%10;redraw=true;}
    if(redraw) {
      volatile uint16_t *guard=(volatile uint16_t*)(SCREEN+38400);
      for(unsigned i=0;i<1280;++i)guard[i]=0xa55a;
      dssmm_background_palette(PALETTE,scene>=5);
      dssmm_background_draw(SCREEN,scene>=5,scene%5+1);
      for(unsigned i=0;i<1280;++i)if(guard[i]!=0xa55a) {
        PALETTE[15]=0x001f;
        for(unsigned j=0;j<19200;++j)((volatile uint16_t*)SCREEN)[j]=0x0f0f;
        while(1){}
      }
      redraw=false;
    }
    while(VCOUNT>=160){}while(VCOUNT<160){}
  }
}
