#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t u32;typedef uint16_t u16;typedef uint8_t u8;typedef volatile uint16_t vu16;typedef volatile uint8_t vu8;
#define MAX_NOR 64
static u16 physical[0x4000000/2],aperture[0x800000/2];
#define FlashBase_S98 ((uintptr_t)aperture)
#define FlashBase_S98_end (FlashBase_S98+0x800000)
static u16 REG_IME;
static u32 gl_currentpage,gl_norOffset;
static struct {char gamename[16];u32 filesize;} pNorFS[MAX_NOR+1];
static void expect(int ok,const char *m){if(!ok){fprintf(stderr,"FAIL: %s\n",m);exit(1);}}
static void SetRompage(u32 p){expect(REG_IME==0,"mapping changed with interrupts masked");expect(p>=gl_currentpage&&(p-gl_currentpage)%0x1000==0&&(p-gl_currentpage)/0x1000<8,"bank within NOR");memcpy(aperture,(u8*)physical+(p-gl_currentpage)/0x1000*0x800000,0x800000);}
#include "omega-scan.impl.h"
static void reset(void){memset(physical,0xff,sizeof(physical));memset(pNorFS,0,sizeof(pNorFS));gl_currentpage=0;gl_norOffset=0;REG_IME=0;}
static void game(unsigned index,u32 start,u32 size){u8 *p=(u8*)physical+start;memcpy(p+0xa0,"TEST ROM    ABCD",16);p[0xb2]=0x96;p[0xbe]=0xce;p[0xbf]=0;memcpy(pNorFS[index].gamename,p+0xa0,16);pNorFS[index].filesize=size;}
static void ready(u16 ime){memcpy(aperture,physical,sizeof(aperture));REG_IME=ime;}
int main(void){
 reset();for(unsigned i=0;i<65;i++)game(i,i*0x20000,0x20000);ready(1);
 expect(GetFileListFromNor()==64&&gl_norOffset==64*0x20000,"catalog scan must stop before slot 64");expect(REG_IME==1,"incoming IRQ state restored");
 reset();game(0,0,0);ready(0);expect(GetFileListFromNor()==0&&gl_norOffset==0,"zero span rejected");expect(REG_IME==0,"disabled IRQ state preserved");
 reset();game(0,0,0xffffffff);ready(1);expect(GetFileListFromNor()==0&&!gl_norOffset,"overflowing catalog span rejected");
 reset();game(0,0,0x20001);ready(1);expect(GetFileListFromNor()==0,"misaligned catalog span rejected");
 reset();game(0,0,0x4000000);ready(1);expect(GetFileListFromNor()==1&&gl_norOffset==0x4000000,"full capacity handled");expect(REG_IME==1,"capacity exit restores IRQ state");
 reset();game(0,0,0x20000);game(1,0x20000,0x20000);ready(1);expect(GetFileListFromNor()==2&&gl_norOffset==0x40000,"ordinary catalog works");expect(GetFileListFromNor()==2&&gl_norOffset==0x40000,"repeated scan recomputes offset");gl_currentpage=0x8000;expect(GetFileListFromNor()==2&&gl_norOffset==0x40000,"kernel base page preserved");
 puts("Omega NOR scan: slot/span/bank bounds and IRQ restoration passed (ROM window simulated).");
}
