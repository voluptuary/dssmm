/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "update_unlock.h"
#include "gbahw.h"
bool update_unlock_feed(update_unlock_state *state,unsigned keys,uint32_t frames) {
  static const uint16_t sequence[]={KEY_BUTTDOWN,KEY_BUTTUP,KEY_BUTTLEFT,KEY_BUTTRIGHT};
  if(!keys)return false;
  if(state->position && (uint64_t)(uint32_t)(frames-state->began)*280896u>5u*16777216u)state->position=0;
  if(keys==sequence[state->position]) {
    if(!state->position)state->began=frames;
    if(++state->position==4){state->position=0;return true;}
  } else {
    state->position=keys==KEY_BUTTDOWN?1:0;
    state->began=frames;
  }
  return false;
}
