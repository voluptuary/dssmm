#include <assert.h>
#include "update_unlock.h"
#include "gbahw.h"
static bool attempt(uint32_t start,uint32_t finish) {
 update_unlock_state s={0};
 assert(!update_unlock_feed(&s,KEY_BUTTDOWN,start));
 assert(!update_unlock_feed(&s,0,start+1)); /* held repeat has no fresh event */
 assert(!update_unlock_feed(&s,KEY_BUTTUP,start+2));
 assert(!update_unlock_feed(&s,KEY_BUTTLEFT,start+3));
 assert(!update_unlock_feed(&s,KEY_BUTTRIGHT,start+4));
 return update_unlock_feed(&s,KEY_BUTTA,finish);
}
int main(void) {
 assert(attempt(100,398));assert(!attempt(100,399));
 assert(attempt(0xffffff00u,30));assert(!attempt(0xffffff00u,50));
 update_unlock_state s={0};
 assert(!update_unlock_feed(&s,KEY_BUTTA,0));assert(!s.position);
 assert(!update_unlock_feed(&s,KEY_BUTTDOWN|KEY_BUTTB|KEY_BUTTSTA,1));assert(!s.position);
 assert(!update_unlock_feed(&s,KEY_BUTTDOWN,2));
 assert(!update_unlock_feed(&s,KEY_BUTTRIGHT,3));assert(!s.position);
 assert(!update_unlock_feed(&s,KEY_BUTTDOWN,4));
 assert(!update_unlock_feed(&s,KEY_BUTTUP,5));
 assert(!update_unlock_feed(&s,KEY_BUTTDOWN,6));assert(s.position==1 && s.began==6);
 assert(!update_unlock_feed(&s,KEY_BUTTUP,7));
 assert(!update_unlock_feed(&s,KEY_BUTTLEFT,8));
 assert(!update_unlock_feed(&s,KEY_BUTTRIGHT,9));
 assert(!update_unlock_feed(&s,KEY_BUTTA|KEY_BUTTB,10));assert(!s.position);
 return 0;
}
