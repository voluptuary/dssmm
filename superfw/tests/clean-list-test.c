#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "util.h"
static void check(const char *in,const char *expected){char out[512];clean_display_name(out,sizeof(out),in);assert(!strcmp(out,expected));}
int main(void){
 check("/Game Boy Advance/P/Pokemon (USA) (En,Fr) [Rev 1].gba","Pokemon");
 check("Game (USA) Deluxe [!].gba","Game Deluxe");check("Game  Two.gbc","Game Two");
 check("Unclosed (name.gba","Unclosed (name");check(".hidden",".hidden");
 check("[USA].gba","[USA].gba");check("Test (one (two)).gba","Test");
 check("Café (Europe).gba","Café");
 char b[4];clean_display_name(b,sizeof(b),"Long title (USA).gba");assert(!strcmp(b,"Lon"));
 char zero='x';clean_display_name(&zero,0,"Game.gba");assert(zero=='x');
 puts("Clean list strips display tags/extensions with bounded output and preserved input.");
}
