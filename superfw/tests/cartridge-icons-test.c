/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include "util.h"
#include "res/icons.h"
#include "res/icon-gamegear.h"
#include "res/icon-pce.h"
#include "icon-lookup-under-test.h"
static void check_icon(const char *path,const uint16_t *tiles,unsigned height) {
 FILE *file=fopen(path,"rb");assert(file);uint16_t raw[256];
 assert(fread(raw,2,height*16,file)==height*16&&fgetc(file)==EOF);fclose(file);
 unsigned native=sizeof(icons_pal)/2,colors=native+sizeof(gamegear_palette)/2,top=(16-height)/2;
 const uint8_t *data=(const uint8_t*)tiles;
 for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x) {
  unsigned tile=(y/8)*2+x/8,index=data[tile*64+(y%8)*8+x%8];assert(index<colors);
  unsigned expected=y<top||y>=top+height?0:raw[(y-top)*16+x];
  if(!expected)assert(index==0);
  else assert((index<native?icons_pal[index]:gamegear_palette[index-native])==expected);
 }
}
int main(int argc,char **argv) {
 assert(argc==3);assert(sizeof(icons_pal)/2+sizeof(gamegear_palette)/2<255);
 assert(sizeof(icons_img)+sizeof(gamegear_pixels)+sizeof(pce_pixels)<=63*256);
 assert(ICON_PCECARD*256==sizeof(icons_img)+sizeof(gamegear_pixels));
 assert(guessicon("/Game Gear/Sonic (USA).GG")==ICON_GGCART);
 assert(guessicon("/SMS/Sonic (USA).sms")==ICON_SMSCART);
 assert(guessicon("/Game Boy/Game.gb")==ICON_GBCART);
 assert(guessicon("/PC Engine/Game (USA).PCE")==ICON_PCECARD);
 check_icon(argv[1],gamegear_pixels,14);check_icon(argv[2],pce_pixels,16);
 puts("Cartridge icon pixels, transparency, shared palettes, sprite bounds and extension routing passed.");
}
