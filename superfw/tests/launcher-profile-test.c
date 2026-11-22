#include <assert.h>
#include <string.h>
#include "launcher_profile.h"
volatile unsigned frame_count;
int main(void) {
 struct {unsigned left;char text[LAUNCHER_NAME_BYTES];unsigned right;} out={0x12345678,{0},0x89abcdef};
 launcher_name_parse(out.text,"ABCDEFGHIJKLM\r\nignored",22);assert(!strcmp(out.text,"ABCDEFGHIJKL"));
 launcher_name_parse(out.text,"\xef\xbb\xbfOwner\nnext",13);assert(!strcmp(out.text,"Owner"));
 launcher_name_parse(out.text,"\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9",24);assert(strlen(out.text)==24);
 launcher_name_parse(out.text,"A\xe2\x82",3);assert(!strcmp(out.text,"A??"));
 launcher_name_parse(out.text,"\xf0\x9f\x8e\xae",4);assert(strlen(out.text)==4);
 launcher_name_parse(out.text,"\xed\xa0\x80",3);assert(!strcmp(out.text,"???"));
 launcher_name_parse(out.text,"",0);assert(!out.text[0]);
 assert(out.left==0x12345678 && out.right==0x89abcdef);
 assert(launcher_clock_get()==UINT32_MAX);
 frame_count=0;launcher_clock_set(86399);frame_count=59;assert(launcher_clock_get()==86399);frame_count=60;assert(launcher_clock_get()==0);
 frame_count=0;launcher_clock_set(0);frame_count=16777216;assert(launcher_clock_get()==21696);
 frame_count=0xfffffff0u;launcher_clock_set(0);frame_count=44;assert(launcher_clock_get()==1);
 launcher_clock_set(UINT32_MAX);frame_count+=120;assert(launcher_clock_get()==UINT32_MAX);
 assert(launcher_title_top(1,1)==58 && launcher_title_top(2,1)==53 && launcher_title_top(3,1)==47);
 assert(launcher_title_top(0,1)==58 && launcher_title_top(99,1)==47);
 assert(launcher_title_top(1,0)==58 && launcher_title_top(2,0)==50 && launcher_title_top(99,0)==50);
 launcher_notice notice={0};
 launcher_notice_update(&notice,100,true);assert(notice.visible && notice.until==160);
 launcher_notice_update(&notice,101,true);assert(notice.until==160);
 launcher_notice_update(&notice,120,false);launcher_notice_update(&notice,121,true);assert(notice.until==163);
 launcher_notice_update(&notice,162,false);assert(notice.visible);
 launcher_notice_update(&notice,163,false);assert(!notice.visible);
 launcher_notice_update(&notice,200,true);assert(notice.until==260);
 launcher_notice_update(&notice,259,false);launcher_notice_update(&notice,260,true);assert(notice.visible && notice.until==263);
 launcher_notice_update(&notice,261,true);assert(notice.until==263);
 launcher_notice_update(&notice,263,false);assert(!notice.visible);
 launcher_notice_update(&notice,0xfffffff0u,true);assert(notice.visible && notice.until==44);
 launcher_notice_update(&notice,43,false);assert(notice.visible);
 launcher_notice_update(&notice,44,false);assert(!notice.visible);
 return 0;
}
