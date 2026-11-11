#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t u32;typedef uint16_t u16;typedef uint8_t u8;typedef uint8_t BYTE;typedef volatile uint16_t vu16;
#define BANK 0x800000U
static u8 physical[0x2000000];
static u16 aperture[(BANK+16)/2];
#define PSRAMBase_S98 ((u8*)aperture)
static u32 selected,is_NORpatch,windows_offset,iTrimSize,gl_rts_on,gl_cheat_on;
static u8 cache_store[0x20000+16];
#define pReadCache cache_store
static void expect(int value,const char *msg){if(!value){fprintf(stderr,"FAIL: %s\n",msg);exit(1);}}
static void SetPSRampage(u16 page){expect(!(page%0x1000)&&page<=0x3000,"valid page");memcpy(physical+selected*BANK,aperture,BANK);selected=page/0x1000;memcpy(aperture,physical+selected*BANK,BANK);}
static void Patch_SpecialROM_TrimSize(void){}
#include "omega-patch.impl.h"
int main(void){
 u16 words[]={0x1234,0x5678,0x9abc};
 memset(physical,0xa7,sizeof(physical));memset(aperture,0xa7,sizeof(aperture));
 Write(BANK-2,(const u8*)words,sizeof(words));
 expect(!memcmp(physical+BANK-2,words,sizeof(words)),"patch split crosses PSRAM bank correctly");expect(selected==0,"default page restored");expect(aperture[BANK/2]==0xa7a7,"no writes beyond aperture");
 Write(1,(const u8*)words,2);expect(physical[1]==0xa7,"odd patch address rejected");
 memset(pReadCache,0xcc,sizeof(pReadCache));is_NORpatch=1;windows_offset=0;
 Write(0x1fffe,(const u8*)words,sizeof(words));
 expect(pReadCache[0x20000]==0xcc&&pReadCache[0x20001]==0xcc,"NOR patch must not overrun staging block");
 expect(!memcmp(pReadCache+0x1fffe,words,2),"first NOR block receives only its part");
 memset(pReadCache,0xcc,sizeof(pReadCache));windows_offset=0x20000;
 Write(0x1fffe,(const u8*)words,sizeof(words));expect(!memcmp(pReadCache,words+1,4)&&pReadCache[4]==0xcc,"next NOR block receives the remaining patch");
 const int offsets[]={-8,0,2,0x1fff8,0x1fffe,0x20000,0x20002};
 const unsigned lengths[]={0,2,4,6};
 for(unsigned a=0;a<sizeof(offsets)/sizeof(offsets[0]);a++)for(unsigned n=0;n<sizeof(lengths)/sizeof(lengths[0]);n++){
  memset(pReadCache,0xcc,sizeof(pReadCache));u32 address=0x20000+offsets[a];Write(address,(const u8*)words,lengths[n]);
  for(unsigned w=0;w<0x10000;w++){
   u32 absolute=0x20000+w*2;u16 expected=0xcccc;
   if(absolute>=address&&absolute-address<lengths[n])expected=words[(absolute-address)/2];
   expect(((u16*)pReadCache)[w]==expected,"NOR block intersection matches reference");
  }
  for(unsigned w=0x20000;w<sizeof(pReadCache);w++)expect(pReadCache[w]==0xcc,"NOR guard bytes preserved");
 }
 is_NORpatch=0;
 memset(pReadCache,0xff,sizeof(pReadCache));
 u32 sizes[]={1,2,3,0xb4,0x20000,0x20001,0x20002,0x20010,0x20020,0x2000000};
 for(unsigned r=0;r<sizeof(sizes)/sizeof(sizes[0]);r++)for(unsigned mode=0;mode<2;mode++)for(unsigned cheats=0;cheats<2;cheats++){
  gl_cheat_on=cheats;gl_rts_on=cheats;iTrimSize=0;
  SetTrimSize(pReadCache,sizes[r],0x20000,mode,0x11);
  expect(iTrimSize>0&&iTrimSize<=sizes[r]+16,"bounded trim result including short block tails");
 }
 iTrimSize=123;SetTrimSize(NULL,100,10,0,0);expect(iTrimSize==123,"null trim ignored");SetTrimSize(pReadCache,0,10,0,0);expect(iTrimSize==123,"empty trim ignored");
 puts("Omega patch/trim boundaries passed with banked-aperture simulation.");
}
