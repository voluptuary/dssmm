#include <assert.h>
#include <string.h>
#include "launcher_profile.h"
volatile unsigned frame_count;
int main(void) {
 struct {unsigned left;char text[LAUNCHER_NAME_BYTES];unsigned right;} out={0x12345678,{0},0x89abcdef};
 launcher_name_parse(out.text,"ABCDEFGHIJKLM\r\nignored",22);assert(!strcmp(out.text,"ABCDEFGHIJK"));
 launcher_name_parse(out.text,"\xef\xbb\xbfOwner\nnext",13);assert(!strcmp(out.text,"Owner"));
 launcher_name_parse(out.text,"\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9",24);assert(strlen(out.text)==22);
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
 return 0;
}
