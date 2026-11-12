
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "kernel_boot_policy.h"
int main(void) {
    assert(KernelPluginRangeFits(0,0x2000000));
    assert(!KernelPluginRangeFits(1,0x2000000));
    assert(!KernelPluginRangeFits(0xFFFFFFFF,2));
    assert(!KernelPluginRangeFits(0x2000000,0x58));
    unsigned char raw[32]={0};
    assert(KernelPluginPayloadValid(raw,0x3FBF8,2));
    assert(!KernelPluginPayloadValid(raw,0x3FBF9,2));
    assert(!KernelPluginPayloadValid(raw,0,2));
    unsigned char literal[]={0x10,4,0,0,0,1,2,3,4};
    assert(KernelPluginPayloadValid(literal,sizeof literal,4));
    assert(!KernelPluginPayloadValid(literal,sizeof literal-1,4));
    unsigned char repeat[]={0x10,4,0,0,0x40,0xA5,0,0};
    assert(KernelPluginPayloadValid(repeat,sizeof repeat,4));
    repeat[6]=0xF0;assert(!KernelPluginPayloadValid(repeat,sizeof repeat,4));
    unsigned char before[]={0x10,3,0,0,0x80,0,0};
    assert(!KernelPluginPayloadValid(before,sizeof before,4));
    literal[0]=0x11;assert(!KernelPluginPayloadValid(literal,sizeof literal,4));
    literal[0]=0x10;literal[1]=0;assert(!KernelPluginPayloadValid(literal,4,4));
    literal[1]=0;literal[2]=0;literal[3]=4;assert(!KernelPluginPayloadValid(literal,4,4));
    unsigned char *fuzz=malloc(4096);assert(fuzz);
    uint32_t seed=1;
    for(unsigned n=0;n<20000;n++) {
        unsigned size=n%4096;
        for(unsigned i=0;i<size;i++) {seed=seed*1664525+1013904223;fuzz[i]=seed>>24;}
        (void)KernelPluginPayloadValid(fuzz,size,4);
    }
    free(fuzz);puts("Plugin payload policy: EWRAM bounds, LZ structure/backreferences and 20,000 malformed cases passed.");
}
