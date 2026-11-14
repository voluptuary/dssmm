#ifndef DSSMM_BROWSER_CAROUSEL_H
#define DSSMM_BROWSER_CAROUSEL_H
#include <stdbool.h>

typedef struct {
  int index;
  int x, y, w, h;
  int distance;
  bool focused;
} browser_carousel_item;

/* Produces the visible neighbors in back-to-front order. */
unsigned browser_carousel_layout(bool vertical, int selected, int count, int offset,
                                 browser_carousel_item items[3]);

#endif
