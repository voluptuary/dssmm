#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint32_t u32;
typedef unsigned UINT;
typedef uint64_t FSIZE_t;
typedef char TCHAR;
typedef struct { unsigned unused; } FIL;
#define IWRAM_CODE
#define FR_OK 0
#define FA_READ 1
#define SRAMSaver 0x0E000000
static unsigned char pReadCache[0x20000], sram[0x20000];
static uint64_t input_size;
static unsigned bank, copied, requested, closed;
static int error_read, short_read, error_close;
int FileTxn_Recover(void) {return 1;}
static int f_open(FIL *file,const char *name,int mode) {(void)file;(void)name;(void)mode;return 0;}
#define f_size(file) input_size
static int f_read(FIL *file,void *data,unsigned bytes,unsigned *read) {
    (void)file;requested=bytes;assert(bytes<=sizeof pReadCache);
    *read=bytes-(short_read && bytes ? 1:0);memset(data,0x5A,*read);return error_read;
}
static int f_close(FIL *file) {(void)file;closed++;return error_close;}
static void SetRampage(unsigned page) {assert(page==0 || page==0x10);bank=page?1:0;}
static void WriteSram(unsigned address,unsigned char *data,unsigned bytes) {
    assert(address==SRAMSaver && bytes<=0x10000);
    memcpy(sram+bank*0x10000,data,bytes);copied+=bytes;
}
#include "save-load.impl.h"
static void reset(uint64_t size) {
    input_size=size;bank=copied=requested=closed=0;error_read=short_read=error_close=0;
    memset(sram,0xCC,sizeof sram);
}
int main(int argc,char **argv) {
    assert(argc==2);
    if(!strcmp(argv[1],"size")) {
        const uint64_t sizes[]={0,1,511,512,0x10000,0x10001,0x20000,0x20001,UINT64_C(0x100000004)};
        for(unsigned i=0;i<sizeof sizes/sizeof sizes[0];i++) {
            reset(sizes[i]);unsigned expected=sizes[i]>sizeof sram?sizeof sram:(unsigned)sizes[i];
            assert(Loadsavefile("game.sav"));
            assert(copied==expected && requested==expected && bank==0 && closed==1);
            for(unsigned j=0;j<sizeof sram;j++) assert(sram[j]==(j<expected?0x5A:0xCC));
        }
    } else {
        for(unsigned i=0;i<3;i++) {
            reset(0x18000);if(i==0)error_read=1;else if(i==1)short_read=1;else error_close=1;
            assert(!Loadsavefile("game.sav") && !copied && closed==1 && bank==0);
            for(unsigned j=0;j<sizeof sram;j++)assert(sram[j]==0xCC);
        }
    }
    puts("Actual save loader: full-width clamp/banks and no SRAM mutation on I/O failure passed.");
}
