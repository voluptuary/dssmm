#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#define LAUNCHER_THUMB_TRANSPARENT_SENTINEL 0x8000
#define IWRAM_CODE
#define ARM_CODE
#define LAUNCHER_MOTION_FAST
static u16 frame[240*160], expected[240*160], host_map[240];
#define VRAM frame
#define LAUNCHER_MOTION_FRAME frame
typedef struct { int x,y,w,h; } LauncherMotionRect;
typedef struct { const u16 *pixels; int w,h; u32 icon,transparent,custom; } LauncherMotionImage;
static u32 launcher_carousel_art_draw=1, enabled=1, rounded, custom;
static u32 Launcher_ArtBorderEnabled(void) { return enabled; }
static u32 Launcher_RoundedCornersForCustom(u32 c) { return rounded == 1 || (rounded == 2 && c); }
static u32 Launcher_RoundedCornersForCarousel(void) { return Launcher_RoundedCornersForCustom(custom); }
static u16 Launcher_ArtBorderColourEx(u32 selected) { return selected ? 0x1234 : 0x4321; }
static u16 Launcher_MotionBorderColour(u32 selected) { return Launcher_ArtBorderColourEx(selected); }
static int Launcher_MotionDivide(int a,int b) { return a/b; }
static void dmaCopy(const void *s,void *d,int n) { memcpy(d,s,n); }
static void Launcher_ClearClip(int x,int y,int w,int h,u16 c) {
 for(int j=y;j<y+h;j++) for(int i=x;i<x+w;i++) if(i>=0&&i<240&&j>=0&&j<160) frame[j*240+i]=c;
}
#include FUNCTIONS_FILE
static u16 source[83*80];
/* Compare both static borders and animated pixels with the pre-contour code.
   The reference uses the effective rounded policy, including CUSTOM Only. */
static void check(int motion,int sw,int sh,int stride,int w,int h,int x,int y,int selected) {
 (void)stride;
 LauncherMotionRect clip={0,0,240,160}, rect={x,y,w,h};
 LauncherMotionImage image={source,sw,sh,0,1,custom};
 memset(frame,0x55,sizeof frame);
 if(motion) Reference_Launcher_MotionBlitCode(clip,rect,image,selected);
 else Reference_Launcher_DrawThumbBorderEx(x,y,w,h,selected);
 memcpy(expected,frame,sizeof frame);
 memset(frame,0x55,sizeof frame);
 if(motion) Launcher_MotionBlitCode(clip,rect,image,selected);
 else Launcher_DrawThumbBorderEx(x,y,w,h,selected);
 if(memcmp(frame,expected,sizeof frame)) { fprintf(stderr,"border mismatch motion=%d size=%dx%d xy=%d,%d round=%u custom=%u\n",motion,w,h,x,y,rounded,custom); exit(1); }
}
int main(void) {
 unsigned cases=0;
 for(int pattern=0;pattern<5;pattern++) for(rounded=0;rounded<3;rounded++) for(custom=0;custom<2;custom++) for(enabled=0;enabled<2;enabled++) for(int motion=0;motion<2;motion++) {
  int stride=motion?80:83;
  for(int y=0;y<80;y++) for(int x=0;x<stride;x++) {
   /* Empty art; disconnected black/high-bit islands; holes; edge-touching art. */
   int solid=pattern==4?1:pattern==1?(x>12&&x<55&&y>8&&y<63)||(x==70&&y==70):pattern==2?(x<80&&!(x>20&&x<50&&y>20&&y<50)):pattern==3?((x+y)%3==0):0;
   source[y*stride+x]=solid?((x&1)?0x8001:0):0x8000;
  }
  const int sizes[][2]={{80,80},{37,37},{79,53},{120,100},{1,1},{240,80}};
  const int origins[][2]={{10,20},{-17,-9},{225,150}};
  for(unsigned z=0;z<sizeof sizes/sizeof *sizes;z++) for(unsigned o=0;o<sizeof origins/sizeof *origins;o++) {
   check(motion,80,80,stride,sizes[z][0],sizes[z][1],origins[o][0],origins[o][1],o&1); cases++;
  }
 }
 printf("Artwork border rollback: %u differential cases passed\n",cases);
 return 0;
}
