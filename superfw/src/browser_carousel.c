#include "browser_carousel.h"
#include "browser_motion.h"

unsigned browser_carousel_layout(bool vertical, int selected, int count, int offset,
                                 browser_carousel_item items[3]) {
  if (!items || count <= 0 || selected < 0 || selected >= count) return 0;
  unsigned total = 0;
  int focused_item = selected, focused_distance = 0x7fffffff;
  for (int slot = -1; slot <= 1; ++slot) {
    int index = selected + slot;
    if (index < 0 || index >= count) continue;
    int position = slot * BROWSER_MOTION_UNIT + offset;
    int distance = position < 0 ? -position : position;
    if (distance < focused_distance) {
      focused_item = index;
      focused_distance = distance;
    }
  }
  for (int slot = -1; slot <= 1; ++slot) {
    int index = selected + slot;
    if (index < 0 || index >= count) continue;
    int position = slot * BROWSER_MOTION_UNIT + offset;
    int distance = position < 0 ? -position : position;
    int w = vertical ? 84 : 120;
    int h = vertical ? 56 : 80;
    int side_w = vertical ? 48 : 60;
    int side_h = vertical ? 32 : 40;
    int d = distance > BROWSER_MOTION_UNIT ? BROWSER_MOTION_UNIT : distance;
    w -= (w - side_w) * d / BROWSER_MOTION_UNIT;
    h -= (h - side_h) * d / BROWSER_MOTION_UNIT;
    int x, y;
    if (vertical) {
      x = 7 + (distance > BROWSER_MOTION_UNIT ? 18 : distance * 18 / BROWSER_MOTION_UNIT);
      y = 62 + position * (position < 0 ? 38 : 46) / BROWSER_MOTION_UNIT;
    } else {
      x = 60 + position * 125 / BROWSER_MOTION_UNIT;
      y = 27 + d * 20 / BROWSER_MOTION_UNIT;
    }
    if (x < 0) x = 0;
    if (x + w > 240) x = 240 - w;
    if (y < 18) y = 18;
    if (y + h > 140) y = 140 - h;
    items[total++] = (browser_carousel_item){
      index, x, y, w, h, distance, index == focused_item
    };
  }
  /* Neighbor rectangles can overlap the focused card, especially vertically. */
  for (unsigned i = 0; i + 1 < total; ++i) {
    if (!items[i].focused) continue;
    browser_carousel_item focus = items[i];
    for (unsigned j = i; j + 1 < total; ++j) items[j] = items[j + 1];
    items[total - 1] = focus;
    break;
  }
  return total;
}
