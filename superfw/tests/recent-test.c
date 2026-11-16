/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "posix-fatfs-fixture.h"
#include "recent.h"
#include "util.h"
void dma_memcpy16(volatile void *dst,const void *src,uint16_t words){memcpy((void*)dst,src,words*2);}
static t_rentry entries[RECENT_MAXFN_CNT],loaded[RECENT_MAXFN_CNT],scratch[RECENT_MAXFN_CNT];
int main(void){char tmp[]="/tmp/dssmm-recents-XXXXXX";assert(mkdtemp(tmp));strcpy(root,tmp);
 unsigned count=0;
 for(unsigned i=0;i<20;i++){char name[256];snprintf(name,sizeof(name),"/Game Boy Advance/%u/Game (USA) %u.gba",i,i);count=insert_recent_fn(entries,count,name,FLAG_RECENT_SD);}
 assert(count==20);count=insert_recent_fn(entries,count,"/NOR Game.gba",FLAG_RECENT_NOR);assert(count==21);
 assert(recent_flush(entries,count));
 char text[10000];read_file(RECENT_FILEPATH,text,sizeof(text));assert(!strstr(text,"nor:"));
 assert(!strncmp(text,"/Game Boy Advance/19/",21));unsigned lines=0;for(char *p=text;*p;p++)lines+=*p=='\n';assert(lines==10);
 unsigned n=recent_load_merged(loaded,scratch);assert(n==21&&!strcmp(loaded[0].fpath,"/NOR Game.gba")&&loaded[0].flags==FLAG_RECENT_NOR);
 write_file(RECENT_FILEPATH,"/External (Europe).gba\r\n/Game Boy Advance/19/Game (USA) 19.gba\r\n");
 n=recent_load_merged(loaded,scratch);assert(n==22&&!strcmp(loaded[0].fpath,"/External (Europe).gba"));
 assert(!strcmp(loaded[1].fpath,"/Game Boy Advance/19/Game (USA) 19.gba"));
 assert(recent_flush(loaded,n));read_file(RECENT_FILEPATH,text,sizeof(text));assert(!strstr(text,"nor:")&&!strchr(text,'\r'));
 unsigned same=insert_recent_fn(loaded,n,"/External (Europe).gba",FLAG_RECENT_SD);assert(same==n&&!strcmp(loaded[0].fpath,"/External (Europe).gba"));
 char before[10000];read_file(PRIVATE_RECENT_FILEPATH,before,sizeof(before));
 short_write=1;assert(!recent_flush(loaded,n));short_write=0;
 read_file(PRIVATE_RECENT_FILEPATH,text,sizeof(text));assert(!strcmp(text,before));
 assert(f_rename(PRIVATE_RECENT_FILEPATH,PRIVATE_RECENT_FILEPATH ".bak")==FR_OK);
 assert(recent_load_merged(loaded,scratch)==n);
 for(unsigned i=0;i<8;i++)assert(!handles[i]);
 f_unlink(PRIVATE_RECENT_FILEPATH ".tmp");
 assert(f_unlink(RECENT_FILEPATH)==FR_OK);assert(f_unlink(PRIVATE_RECENT_FILEPATH)==FR_OK);
 char q[1024];path(q,"/SYSTEM");assert(!rmdir(q));assert(!rmdir(root));
 puts("Recent writes create SYSTEM, portable SD newest-first10, private NOR history, DE reorder/CRLF and failures passed.");
}
