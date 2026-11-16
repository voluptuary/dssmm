/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include "util.h"
#include "res/icons.h"
#include "res/icon-gamegear.h"
#include "icon-lookup-under-test.h"
int main(int argc,char **argv){
 assert(argc==2);FILE *file=fopen(argv[1],"rb");assert(file);uint16_t raw[224];
 assert(fread(raw,2,224,file)==224&&fgetc(file)==EOF);fclose(file);
 unsigned native=sizeof(icons_pal)/2,extra=sizeof(gamegear_palette)/2;
 assert(native+extra<255);assert(sizeof(icons_img)+sizeof(gamegear_pixels)<=63*256);
 assert(guessicon("/Game Gear/Sonic (USA).GG")==ICON_GGCART);
 assert(guessicon("/SMS/Sonic (USA).sms")==ICON_SMSCART);
 assert(guessicon("/Game Boy/Game.gb")==ICON_GBCART);
 const uint8_t *data=(const uint8_t*)gamegear_pixels;
 for(unsigned y=0;y<16;y++)for(unsigned x=0;x<16;x++){
  unsigned tile=(y/8)*2+x/8,index=data[tile*64+(y%8)*8+x%8];assert(index<native+extra);
  unsigned expected=y==0||y==15?0:raw[(y-1)*16+x];
  if(!expected)assert(index==0);
  else assert((index<native?icons_pal[index]:gamegear_palette[index-native])==expected);
 }
 puts("Game Gear pixels/padding, native palette IDs, sprite budget and extension icon lookup passed.");
}
