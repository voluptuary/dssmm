#ifndef DSSMM_BROWSER_CAROUSEL_H
#define DSSMM_BROWSER_CAROUSEL_H
#include <stdbool.h>

typedef struct {
  int index;
  int x, y, w, h;
  int distance;
  bool focused;
} browser_carousel_item;

static inline int browser_carousel_index(int index,int count,bool wrap) {
  if(count<=0)return -1;
  if(!wrap)return index<0 || index>=count?-1:index;
  index%=count;return index<0?index+count:index;
}
static inline int browser_carousel_step(int selected,int count,int direction,bool wrap) {
  int index=browser_carousel_index(selected+direction,count,wrap);
  return index<0?selected:index;
}
/* Produces the visible neighbors in back-to-front order. */
unsigned browser_carousel_layout(bool vertical, int selected, int count, int offset, bool wrap,
                                 browser_carousel_item items[3]);

#endif
