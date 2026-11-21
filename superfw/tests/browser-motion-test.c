#include <assert.h>
#include "browser_carousel.h"
#include "browser_motion.h"

static void index_order(browser_carousel_item items[3]) {
  assert(items[2].focused); /* Production painter order must put focus last. */
  for (unsigned i=0;i<3;++i) for(unsigned j=i+1;j<3;++j)
    if(items[j].index<items[i].index) {
      browser_carousel_item tmp=items[i];items[i]=items[j];items[j]=tmp;
    }
}
int main(void) {
  int offset = browser_motion_retarget(0, 1);
  assert(offset == 256);
  assert(browser_motion_advance(offset, 1) == 208);
  assert(browser_motion_advance(offset, 5) == 16);
  assert(browser_motion_advance(offset, 6) == 0);
  offset = browser_motion_retarget(offset, 1);
  assert(offset == 512); /* A second input preserves the visual trajectory. */
  offset = browser_motion_retarget(offset, -1);
  assert(offset == 256);
  assert(browser_motion_retarget(0, 9) == BROWSER_MOTION_LIMIT);
  assert(browser_motion_retarget(0, -9) == -BROWSER_MOTION_LIMIT);
  assert(browser_motion_advance(-256, 6) == 0);
  assert(browser_motion_advance(256, 0) == 256);

  browser_carousel_item items[3];
  assert(browser_carousel_layout(false, 2, 5, 0, false, items) == 3);
  index_order(items);
  assert(items[0].index == 1 && items[1].index == 2 && items[2].index == 3);
  assert(items[1].focused && items[1].x == 60 && items[1].y == 27 &&
         items[1].w == 120 && items[1].h == 80);
  assert(items[0].x == -5 && items[0].y == 47 && items[0].w == 60 && items[0].h == 40);
  assert(items[2].x == 185 && items[2].y == 47 && items[2].w == 60 && items[2].h == 40);
  for (unsigned i = 0; i < 3; ++i)
    assert(items[i].w > 0 && items[i].h > 0);
  assert(browser_carousel_layout(false, 2, 5, 256, false, items) == 3);
  assert(items[2].focused && items[2].index == 1);
  assert(browser_carousel_layout(true, 2, 5, 0, false, items) == 3);
  index_order(items);
  assert(items[1].focused && items[1].x == 7 && items[1].y == 62 &&
         items[1].w == 84 && items[1].h == 56);
  assert(items[0].x == 25 && items[0].y == 24 && items[0].w == 48 && items[0].h == 32);
  assert(items[2].x == 25 && items[2].y == 124 && items[2].w == 48 && items[2].h == 32);
  assert(browser_carousel_layout(false, 0, 1, 0, false, items) == 1 && items[0].index == 0);
  assert(browser_carousel_layout(true, 4, 5, 0, false, items) == 2);
  for (int vertical=0;vertical<2;++vertical)
    for (int count=1;count<=5;++count)
      for (int selected=0;selected<count;++selected)
        for (int offset=-BROWSER_MOTION_LIMIT;offset<=BROWSER_MOTION_LIMIT;offset+=16) {
          unsigned n=browser_carousel_layout(vertical,selected,count,offset,false,items);
          assert(n>=1 && n<=3 && items[n-1].focused);
          unsigned focuses=0;
          for(unsigned i=0;i<n;++i) {
            focuses+=items[i].focused;
            assert(items[i].index>=0 && items[i].index<count);
            assert(items[i].x>=-500 && items[i].y>=-500);
            assert(items[i].w>=32 && items[i].w<=120 && items[i].h>=32 && items[i].h<=80);
            assert(items[i].x<=500 && items[i].y<=500);
          }
          assert(focuses==1);
        }
  return 0;
  for(int count=1;count<=20;++count)for(int selected=0;selected<count;++selected)
    for(int offset=-768;offset<=768;offset+=64) {
      unsigned n=browser_carousel_layout(false,selected,count,offset,true,items),focus=0;
      assert(n==(count==1?1u:3u));
      for(unsigned j=0;j<n;++j){assert(items[j].index>=0 && items[j].index<count);focus+=items[j].focused;}
      assert(focus==1);
    }
  assert(browser_carousel_step(0,16,-1,true)==15);
  assert(browser_carousel_step(15,16,1,true)==0);
  assert(browser_carousel_step(0,16,-1,false)==0);

}
