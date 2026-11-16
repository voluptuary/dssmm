/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "posix-fatfs-fixture.h"
#include "patchengine.h"
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define WRITE_LOG(...) ((void)0)
#define MAPPED_SDRAM 1
static uint8_t hiscratch[8*1024*1024] __attribute__((aligned(4)));
static bool sd_mapped=true,scan_ok=true;
static unsigned scans,writes,finalized,file_size;
FRESULT f_lseek(FIL *file,FSIZE_t offset){return fseek(handles[file->obj.id],offset,SEEK_SET)?FR_DISK_ERR:FR_OK;}
static void set_supercard_mode(unsigned area,bool writable,bool sd){assert(area==MAPPED_SDRAM&&writable);sd_mapped=sd;}
static void dma_memcpy32(void *dst,const void *src,unsigned words){assert(!sd_mapped);memcpy(dst,src,words*4);}
static void loadrom_progress(unsigned done,unsigned total){(void)done;assert(total);}
void patchengine_init(t_patch_builder *builder,unsigned size){memset(builder,0,sizeof(*builder));builder->filesize=size;}
void patchengine_finalize(t_patch_builder *builder){(void)builder;++finalized;}
bool write_patches_cache(const char *filename,const t_patch *patch){(void)filename;(void)patch;++writes;return true;}
bool patchengine_process_window(const uint32_t *data,unsigned available,unsigned begin,unsigned end,t_patch_builder *builder,void(*progress)(unsigned)){
 assert(!sd_mapped&&begin<end&&end<=available&&builder->rom_offset+available<=file_size);++scans;
 for(unsigned i=0;i<available;i++)assert(((uint8_t*)data)[i]==(uint8_t)((builder->rom_offset+i)*17+3));
 if(progress)progress(begin/4);return scan_ok;
}
#include "patch-load-under-test.h"
static void check_closed(void){assert(sd_mapped);for(unsigned i=0;i<8;i++)assert(!handles[i]);}
int main(void) {
 char temp[]="/tmp/dssmm-patch-load-XXXXXX";assert(mkdtemp(temp));strcpy(root,temp);
 char name[1024];path(name,"/test.gba");FILE *file=fopen(name,"wb");assert(file);
 file_size=9*1024*1024+3;
 for(unsigned i=0;i<file_size;i++)assert(fputc((uint8_t)(i*17+3),file)!=EOF);assert(!fclose(file));
 assert(generate_patches_progress("/test.gba",file_size));assert(scans==2&&writes==1&&finalized==1);check_closed();
 assert(!generate_patches_progress("/test.gba",file_size-1));check_closed();
 read_error=1;assert(!generate_patches_progress("/test.gba",file_size));read_error=0;check_closed();
 short_read=1;assert(!generate_patches_progress("/test.gba",file_size));short_read=0;check_closed();
 scan_ok=false;assert(!generate_patches_progress("/test.gba",file_size));check_closed();
 assert(writes==1&&finalized==1);assert(f_unlink("/test.gba")==FR_OK);assert(!rmdir(root));
 puts("Production patch loader: overlapping exact reads, short/error handling, restored SD mapping and no partial cache writes passed.");
}
