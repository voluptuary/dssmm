#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "artwork.h"
static uint8_t bmp[ARTWORK_BMP_MAX_SIZE];
static uint16_t pixels[ARTWORK_MAX_PIXELS/2+8];
static unsigned bytes;
static bool read_image(void *ctx,uint32_t off,void *out,size_t size){(void)ctx;if(off>bytes || size>bytes-off)return false;memcpy(out,bmp+off,size);return true;}
int main(int argc,char **argv){assert(argc==2);unsigned count=0;
 for(unsigned mode=0;mode<2;++mode)for(unsigned i=0;i<27;++i){
  char name[1024],letter=i?'A'+i-1:'#';unsigned width=mode?80:120;
  assert(snprintf(name,sizeof(name),"%s/SYSTEM/%s/CUSTOM/%c.bmp",argv[1],mode?"IMGS2":"IMGS",letter)<sizeof(name));
  FILE *f=fopen(name,"rb");assert(f);bytes=fread(bmp,1,sizeof(bmp),f);assert(fgetc(f)==EOF);fclose(f);
  for(unsigned j=0;j<sizeof(pixels)/2;++j)pixels[j]=0xa55a;
  artwork_info info;assert(artwork_decode(read_image,0,bytes,(uint8_t *)pixels,ARTWORK_MAX_PIXELS,&info));
  assert(info.width==width && info.height==80 && info.transparent);
  uint8_t *data=(uint8_t *)pixels;assert(data[0]==0 && data[width*80-1]==0);
  unsigned transparent=0,opaque=0;bool colors[256]={0};
  for(unsigned j=0;j<width*80;++j){colors[data[j]]=true;if(data[j])++opaque;else ++transparent;}
  assert(opaque>2000 && transparent>500);unsigned unique=0;for(unsigned j=0;j<256;++j)unique+=colors[j];assert(unique>=5);
  for(unsigned j=ARTWORK_MAX_PIXELS/2;j<sizeof(pixels)/2;++j)assert(pixels[j]==0xa55a);
  ++count;
 }
 assert(count==54);puts("All54 Title/Box letter-folder BMPs decode with DST1 transparency and guarded output.");}
