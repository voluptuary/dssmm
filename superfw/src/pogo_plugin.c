/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <string.h>
#include "pogo_plugin.h"

#pragma GCC optimize ("Os")
#define ROM_BASE 0x08000000u
/* Keep clear of the existing menu/font resource region beginning at 14MiB. */
#define PAYLOAD_LIMIT (14u*1024*1024)
bool pogo_layout(uint32_t ps,uint32_t fs,uint32_t *fo,uint32_t *to) {
  if (!ps || !fs || ps>PAYLOAD_LIMIT || fs>PAYLOAD_LIMIT) return false;
  uint32_t start=(ps+3)&~3u;
  if (fs>PAYLOAD_LIMIT-start) return false;
  uint32_t table=(start+fs+32767)&~32767u;
  if (table>PAYLOAD_LIMIT-88) return false;
  *fo=start;*to=table;return true;
}
/* SMSAdvance/PCEAdvance write WAITCNT=0x14 on entry, which makes cartridge
 * reads too fast for SuperCard SDRAM. Match the complete ARM instruction chain
 * and its in-block literal; change only the MOV value, never the register or a
 * guessed data constant. On-disk plugins and game bytes remain untouched. */
bool pogo_fix_waitcnt(uint32_t *data,uint32_t size) {
  for(unsigned i=0;i+12<=size;i+=4) {
    unsigned n=i/4,ldr=data[n+1];
    if(data[n]!=0xe3a00014 || (ldr&0xfffff000)!=0xe59f1000 || data[n+2]!=0xe1c100b0)continue;
    unsigned literal=i+12+(ldr&4095);
    if((literal&3) || literal>size-4 || data[literal/4]!=0x04000204)continue;
    volatile uint16_t *patch=(volatile uint16_t*)&data[n];
    patch[0]=0x0901;patch[1]=0xe3a0; /* mov r0,#0x4000: default waits plus prefetch */
    return true;
  }
  return false;
}
static void short_name(char out[32],const char *name) {
  const char *base=strrchr(name,'/');base=base?base+1:name;
  size_t n=strlen(base);memset(out,0,32);
  if(n<=31)memcpy(out,base,n);
  else {
    const char *ext=strrchr(base,'.');size_t e=ext?strlen(ext):0;
    if(e>31){memcpy(out,ext,31);return;}
    memcpy(out,base,31-e);if(e)memcpy(out+31-e,ext,e);
  }
}
void pogo_arguments(uint32_t ram[24],uint32_t table[22],const char *plugin,
                    const char *filename,uint32_t fo,uint32_t fs,uint32_t to) {
  char cmd[32],file[32];short_name(cmd,plugin);short_name(file,filename);
  memset(ram,0,96);memset(table,0,88);
  /* ram starts at 0x0203fbf8; argc/magic live at 0x0203fc00. */
  ram[0]=fs;ram[1]=ROM_BASE+fo;ram[2]=0xfab0babe;ram[3]=2;
  char *p=(char*)&ram[4];*p++='/';memcpy(p,cmd,strlen(cmd)+1);p+=strlen(cmd)+1;
  *p++='/';memcpy(p,file,strlen(file)+1);
  table[0]=0xfab0babe;table[1]=0x80000050;
  memcpy(table+2,cmd,32);table[10]=fo;table[11]=0u-(to+8);
  memcpy(table+12,file,32);table[20]=fs;table[21]=fo-(to+8);
}

/* Validate the entire BIOS LZ77 stream before handing it to SWI 0x11. */
bool pogo_ram_valid(const uint8_t *data,uint32_t size,unsigned mode) {
  if(mode==0)return true;
  if(mode==1)return size && !(size&3) && size<=0x3fbf8;
  if(mode!=2 || size<4 || data[0]!=0x10)return false;
  uint32_t out=data[1]|((uint32_t)data[2]<<8)|((uint32_t)data[3]<<16);
  if(!out || out>0x3fbf8)return false;
  uint32_t ip=4,op=0;
  while(op<out) {
    if(ip==size)return false;
    unsigned flags=data[ip++];
    for(unsigned bit=0;bit<8 && op<out;bit++) {
      if(flags&(128>>bit)) {
        if(size-ip<2)return false;
        unsigned a=data[ip++],b=data[ip++],len=(a>>4)+3,dist=((a&15)<<8)+b+1;
        if(dist>op || len>out-op)return false;
        op+=len;
      }else {if(ip==size)return false;++ip;++op;}
    }
  }
  return true;
}

/* Complete known FluBBa Visoly doReset bodies; never guess from a lone SWI. */
static const uint32_t reset_sms[]={0xe3a01301,0xe3a00000,0xe1c10bba,0xe1c10cb6,
  0xe1c10db2,0xe1c10dbe,0xe5810208,0xebffffd6,0xe3a00000,0xe59f1090,
  0xe1c100b0,0xebfffff0,0xe3a0000c,0xef010000,0xef000000};
static const uint32_t reset_pce[]={0xe3a01301,0xe3a00000,0xe1c10bba,0xe1c10cb6,
  0xe1c10db2,0xe1c10dbe,0xe2811c02,0xe5810008,0xebffffd3,0xe3a00000,
  0xe59f1084,0xe1c100b0,0xebffffef,0xe3a0000c,0xef010000,0xef000000};
unsigned pogo_adapt_image(uint32_t *data,uint32_t size,uint32_t target) {
  pogo_fix_waitcnt(data,size);
  if(!target || (target&3))return 0;
  unsigned patched=0;
  for(unsigned i=0;i+sizeof(reset_sms)<=size;i+=4) {
    const void *body=(const uint8_t*)data+i;
    bool sms=!memcmp(body,reset_sms,sizeof(reset_sms));
    bool pce=i+sizeof(reset_pce)<=size && !memcmp(body,reset_pce,sizeof(reset_pce));
    if(!sms && !pce)continue;
    volatile uint16_t *p=(volatile uint16_t*)body;
    p[0]=0xf004;p[1]=0xe51f; /* ldr pc,[pc,#-4] */
    p[2]=target;p[3]=target>>16;++patched;
  }
  return patched;
}
bool pogo_return_available(const uint32_t *image,const uint32_t *stub,uint32_t version,uint32_t limit) {
  return image[0xa0/4]==0x45505553 && image[0xa4/4]==0x00574652 &&
      image[0xc8/4]==version && image[0xcc/4]>=256 && image[0xcc/4]<=limit &&
      !(image[0xcc/4]&511) && stub[1]==0x54525344;
}
/* LZ77 output uses only halfword writes, including overlapping backreferences. */
unsigned pogo_unpack16(const uint8_t *in,uint32_t size,uint16_t *out) {
  if(!pogo_ram_valid(in,size,2))return 0;
  unsigned total=in[1]|((unsigned)in[2]<<8)|((unsigned)in[3]<<16),ip=4,op=0;
  volatile uint16_t *dest=out;
  while(op<total) {
    unsigned flags=in[ip++];
    for(unsigned bit=0;bit<8 && op<total;++bit) {
      unsigned length=1,distance=0;
      if(flags&(128>>bit)) {
        unsigned a=in[ip++],b=in[ip++];length=(a>>4)+3;distance=((a&15)<<8)+b+1;
      }
      while(length--) {
        unsigned value=distance?(((const uint8_t*)out)[op-distance]):in[ip++];
        unsigned shift=(op&1)*8;
        dest[op/2]=(dest[op/2]&~(255u<<shift))|(value<<shift);++op;
      }
    }
  }
  /* Reset copies complete words; the last output word has deterministic padding. */
  for(unsigned i=op;i<((total+3)&~3u);++i) {
    unsigned shift=(i&1)*8;dest[i/2]&=~(255u<<shift);
  }
  return total;
}
