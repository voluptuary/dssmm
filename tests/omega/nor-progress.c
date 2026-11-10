#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint32_t u32;typedef uint16_t u16;
#define NOR_MENU_WIDTH 168
#define NOR_PROGRESS_X 72
#define NOR_PROGRESS_Y 104
#define NOR_PROGRESS_W 96
#define NOR_PROGRESS_H 8
static u16 gImage_MENU[168*110];
static u32 gl_color_selectBG_sd=0x1234,gl_color_text=0x2345;
static unsigned refreshes,fills;
static char label[16];
static void Launcher_UpdateNORWriteClock(unsigned force){(void)force;refreshes++;}
static void DrawPic(u16 *p,int x,int y,int w,int h,int a,int b,int c){(void)a;(void)b;(void)c;assert(p>=gImage_MENU && p+w<=gImage_MENU+168*110);assert(x==36&&y>=82&&y<94&&w==168&&h==1);}
static void DrawHZText12(char *s,int n,int x,int y,u32 color,int direct){(void)n;(void)direct;assert(y==82&&x>=0&&x<240&&color==gl_color_text);snprintf(label,sizeof(label),"%s",s);}
static void Clear(int x,int y,int w,int h,u32 color,int direct){(void)direct;assert(x==72&&y==104&&w>0&&w<=96&&h==8);assert(color==gl_color_text||color==gl_color_selectBG_sd);fills++;}
#include "omega-progress.impl.h"
int main(void){
 for(u32 total=1;total<=512;total++){
  u32 last=101;refreshes=fills=0;
  for(u32 done=0;done<=total;done++){
   NOR_DrawWriteProgress(done,total,&last);assert(last==done*100/total);
   unsigned saved=refreshes;NOR_DrawWriteProgress(done,total,&last);assert(saved==refreshes);
  }
  assert(!strcmp(label,"100%"));assert(refreshes==(total<100?total+1:101));assert(fills>=refreshes&&fills<=refreshes*2);
 }
 puts("Omega NOR progress: all 1..512-block spans, redraw cadence and drawing bounds passed.");
}
