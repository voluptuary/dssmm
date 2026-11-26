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
#include "res/icon-ngp.h"
#include "res/icon-ngpc.h"
#include "res/icon-a26.h"
#include "icon-lookup-under-test.h"
static void check_icon(const char *path,const uint16_t *tiles,unsigned height) {
 FILE *file=fopen(path,"rb");assert(file);uint16_t raw[256];
 assert(fread(raw,2,height*16,file)==height*16&&fgetc(file)==EOF);fclose(file);
 unsigned native=sizeof(icons_pal)/2,colors=native+sizeof(gamegear_palette)/2,top=(16-height)/2;
 const uint8_t *data=(const uint8_t*)tiles;
 unsigned left=16,right=0,top_pixel=16,bottom=0;
 for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x) {
  unsigned tile=(y/8)*2+x/8,index=data[tile*64+(y%8)*8+x%8];assert(index<colors);
  unsigned expected=y<top||y>=top+height?0:raw[(y-top)*16+x];
  if(!expected)assert(index==0);
  else {
   assert((index<native?icons_pal[index]:gamegear_palette[index-native])==expected);
   if(x<left)left=x;if(x>right)right=x;if(y<top_pixel)top_pixel=y;if(y>bottom)bottom=y;
  }
 }
 if(strstr(path,"/pce."))assert(right-left+1==9 && bottom-top_pixel+1==14);
 if(strstr(path,"/ngp.")||strstr(path,"/ngpc."))assert(right-left+1==14 && bottom-top_pixel+1==12);
 if(strstr(path,"/a26."))assert(right-left+1==12 && bottom-top_pixel+1==14);
}
int main(int argc,char **argv) {
 assert(argc==6);assert(sizeof(icons_pal)/2+sizeof(gamegear_palette)/2<255);
 assert(sizeof(icons_img)+sizeof(gamegear_pixels)+sizeof(pce_pixels)+sizeof(ngp_pixels)+sizeof(ngpc_pixels)+sizeof(a26_pixels)<=63*256);
 assert(ICON_PCECARD*256==sizeof(icons_img)+sizeof(gamegear_pixels));
 assert(guessicon("/Game Gear/Sonic (USA).GG")==ICON_GGCART);
 assert(guessicon("/SMS/Sonic (USA).sms")==ICON_SMSCART);
 assert(guessicon("/Game Boy/Game.gb")==ICON_GBCART);
 assert(guessicon("/PC Engine/Game (USA).PCE")==ICON_PCECARD);
 assert(ICON_NGPCART==ICON_PCECARD+1 && ICON_NGPCCART==ICON_NGPCART+1);
 assert(guessicon("/Neo Geo Pocket/Game (Japan).NGP")==ICON_NGPCART);
 assert(guessicon("/Neo Geo Pocket Color/Game (USA).ngc")==ICON_NGPCCART);
 assert(guessicon("Game.NGPC")==ICON_NGPCCART);
 assert(ICON_A26CART==ICON_NGPCCART+1);
 assert(guessicon("/Atari 2600/Game.A26")==ICON_A26CART);
 check_icon(argv[1],gamegear_pixels,14);check_icon(argv[2],pce_pixels,16);
 check_icon(argv[3],ngp_pixels,16);check_icon(argv[4],ngpc_pixels,16);check_icon(argv[5],a26_pixels,16);
 puts("Cartridge icon pixels, transparency, shared palettes, sprite bounds and extension routing passed.");
}
