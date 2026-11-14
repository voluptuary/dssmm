/* DSSMM carousel motion state shared by SD, Lite and Chis. */
#ifndef DSSMM_BROWSER_MOTION_H
#define DSSMM_BROWSER_MOTION_H

enum { BROWSER_MOTION_UNIT = 256, BROWSER_MOTION_STEP = 48, BROWSER_MOTION_LIMIT = 3 * 256 };

static inline int browser_motion_retarget(int offset, int selection_delta) {
  int next = offset + selection_delta * BROWSER_MOTION_UNIT;
  if (next > BROWSER_MOTION_LIMIT) next = BROWSER_MOTION_LIMIT;
  if (next < -BROWSER_MOTION_LIMIT) next = -BROWSER_MOTION_LIMIT;
  return next;
}

static inline int browser_motion_advance(int offset, unsigned frames) {
  unsigned step = frames * BROWSER_MOTION_STEP;
  if (!offset || !frames) return offset;
  if (step >= (unsigned)(offset < 0 ? -offset : offset)) return 0;
  return offset < 0 ? offset + (int)step : offset - (int)step;
}

#endif
