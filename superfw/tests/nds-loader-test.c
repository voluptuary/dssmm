/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "posix-fatfs-fixture.h"
#include <sys/mman.h>
#include "../src/nds_loader.c"
FRESULT f_lseek(FIL *file,FSIZE_t offset){return fseek(handles[file->obj.id],offset,SEEK_SET)?FR_DISK_ERR:FR_OK;}
void memset32(void *dst,uint32_t value,unsigned size){assert(!value);memset(dst,0,size);}
void memcpy32(void *restrict dst,const void *restrict src,unsigned size){memcpy(dst,src,size);}
int dldi_stub_find(const void *buffer,unsigned size){(void)buffer;(void)size;return -1;}
bool dldi_stub_validate(const t_dldi_header *header,unsigned size){(void)header;(void)size;return false;}
void dldi_stub_patch(t_dldi_driver *stub,const t_dldi_driver *driver){(void)stub;(void)driver;abort();}
static void write_header(const t_nds_header *header) {
 char name[1024];path(name,"/test.nds");FILE *file=fopen(name,"wb");assert(file);
 assert(fwrite(header,1,sizeof(*header),file)==sizeof(*header));assert(!fclose(file));
}
static void no_handles(void){for(unsigned i=0;i<8;++i)assert(!handles[i]);}
int main(void) {
 char temp[]="/tmp/dssmm-nds-audit-XXXXXX";assert(mkdtemp(temp));strcpy(root,temp);
 void *ram=mmap((void*)0x02000000,0x800000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0);assert(ram==(void*)0x02000000);
 t_nds_header header={0};header.arm9_load_addr=header.arm9_entrypoint=MAINRAM_MIN_ADDR+4096;
 header.arm7_load_addr=header.arm7_entrypoint=MAINRAM_MIN_ADDR+8192;
 write_header(&header);assert(load_nds("/test.nds",0)==0);no_handles();
 read_error=1;assert(load_nds("/test.nds",0)==ERR_FILE_ACCESS);read_error=0;no_handles();
 t_dldi_driver driver={0};driver.h.req_size=32;
 assert(load_nds("/test.nds",&driver)==ERR_NDS_BADHEADER);no_handles();
 header.arm9_load_size=MAINRAM_MAX_PAYLOAD+1;write_header(&header);
 assert(load_nds("/test.nds",0)==ERR_NDS_TOO_BIG);no_handles();
 header.arm9_load_size=4;header.arm9_rom_offset=512;write_header(&header);
 assert(load_nds("/test.nds",0)==ERR_FILE_ACCESS);no_handles();
 assert(!munmap(ram,0x800000));assert(f_unlink("/test.nds")==FR_OK);assert(!rmdir(root));
 puts("NDS loader: optional driver, invalid driver size, read/header/payload failures and handle closure passed (synthetic RAM only).");
}
