/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <string.h>
#include "pogo_plugin.h"
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
    data[n]=0xe3a00901; /* mov r0,#0x4000: default waits plus prefetch */
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
