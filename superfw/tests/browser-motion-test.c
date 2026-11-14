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
  assert(browser_carousel_layout(false, 2, 5, 0, items) == 3);
  index_order(items);
  assert(items[0].index == 1 && items[1].index == 2 && items[2].index == 3);
  assert(items[1].focused && items[1].x == 60 && items[1].y == 27 &&
         items[1].w == 120 && items[1].h == 80);
  assert(items[0].x == 0 && items[0].y == 47 && items[0].w == 60 && items[0].h == 40);
  assert(items[2].x == 180 && items[2].y == 47 && items[2].w == 60 && items[2].h == 40);
  for (unsigned i = 0; i < 3; ++i)
    assert(items[i].x >= 0 && items[i].y >= 18 && items[i].x + items[i].w <= 240 && items[i].y + items[i].h <= 140);
  assert(browser_carousel_layout(false, 2, 5, 256, items) == 3);
  assert(items[2].focused && items[2].index == 1);
  assert(browser_carousel_layout(true, 2, 5, 0, items) == 3);
  index_order(items);
  assert(items[1].focused && items[1].x == 7 && items[1].y == 62 &&
         items[1].w == 84 && items[1].h == 56);
  assert(items[0].x == 25 && items[0].y == 24 && items[0].w == 48 && items[0].h == 32);
  assert(items[2].x == 25 && items[2].y == 108 && items[2].w == 48 && items[2].h == 32);
  assert(browser_carousel_layout(false, 0, 1, 0, items) == 1 && items[0].index == 0);
  assert(browser_carousel_layout(true, 4, 5, 0, items) == 2);
  return 0;
}
