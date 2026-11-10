#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ff.h"
typedef uint32_t u32;typedef uint8_t u8;typedef char TCHAR;
#define MAX_pReadCache_size 0x20000
static FIL gfile;
static u8 pReadCache[MAX_pReadCache_size];
static FSIZE_t size;
static unsigned opens,closes,seeks,reads,trimmed;
static int open_error,seek_error,read_error,short_read,close_error;
static UINT requested;
static FSIZE_t offset;
static void expect(int value,const char *msg){if(!value){fprintf(stderr,"FAIL: %s\n",msg);exit(1);}}
FRESULT f_open(FIL *f,const TCHAR *name,BYTE mode){(void)name;(void)mode;opens++;memset(f,0,sizeof(*f));f->obj.objsize=size;return open_error?FR_NO_FILE:FR_OK;}
FRESULT f_lseek(FIL *f,FSIZE_t p){(void)f;seeks++;offset=p;return seek_error?FR_DISK_ERR:FR_OK;}
FRESULT f_read(FIL *f,void *out,UINT n,UINT *got){(void)f;reads++;requested=n;*got=short_read&&n?n-1:n;memset(out,0x5a,*got);return read_error?FR_DISK_ERR:FR_OK;}
FRESULT f_close(FIL *f){(void)f;closes++;return close_error?FR_DISK_ERR:FR_OK;}
static void SetTrimSize(u8 *p,u32 romsize,u32 span,u32 mode,BYTE save){(void)romsize;(void)mode;(void)save;expect(p==pReadCache&&span==MAX_pReadCache_size&&p[0]==0x5a,"trim called with valid tail data");trimmed++;}
#include "omega-trim.impl.h"
static void reset(void){size=0x20001;opens=closes=seeks=reads=trimmed=0;open_error=seek_error=read_error=short_read=close_error=0;}
int main(void){
 reset();expect(Launcher_LoadTrimSize("Test.gba",0x20001,0,0x11),"one-byte final tail accepted");expect(requested==1&&offset==0x20000&&trimmed==1&&closes==1,"only existing tail byte read");
 reset();size=0x20000;expect(Launcher_LoadTrimSize("Test.gba",size,1,0x11)&&requested==0x20000&&offset==0,"exact block read");
 for(int fault=0;fault<5;fault++){
  reset();if(fault==0)open_error=1;if(fault==1)seek_error=1;if(fault==2)read_error=1;if(fault==3)short_read=1;if(fault==4)close_error=1;
  expect(!Launcher_LoadTrimSize("Test.gba",0x20001,0,0x11)&&!trimmed,"failed I/O cannot analyze stale tail");expect(closes==(fault==0?0:1),"opened file always closed exactly once");
 }
 reset();size=((FSIZE_t)1<<32)+0x20001;expect(!Launcher_LoadTrimSize("Test.gba",0x20001,0,0x11)&&!reads&&closes==1,"size mismatch cannot narrow/wrap");
 reset();expect(!Launcher_LoadTrimSize("Test.gba",0,0,0)&&!opens,"empty input rejected");expect(!Launcher_LoadTrimSize(NULL,0x20001,0,0)&&!opens,"null name rejected");expect(!Launcher_LoadTrimSize("Test.gba",0x20001,2,0)&&!opens,"invalid mode rejected");
 puts("Omega trim I/O: exact tail bytes, size changes and open/seek/read/close failures passed.");
}
