#!/usr/bin/env python3
"""Exercise actual native/plugin probes, copy failures and BLUE-only update validation."""
from pathlib import Path
import os
import subprocess
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[1]
main=(ROOT/'targets/blue/source/ezbluekernel.c').read_text()
copy_source=main
if len(sys.argv)>1:
    copy_source=subprocess.check_output(['git','show',sys.argv[1]+':source/ezkernelnew.c'],text=True)
copy=copy_source[copy_source.index('u32 Copy_file('):copy_source.index('u32 Is_bin_file(')]
adapter=(ROOT/'targets/blue/source/blue_frontend_adapter.c').read_text()
candidate=adapter[adapter.index('u8 Blue_ValidateKernelCandidate('):adapter.index('void Blue_BootNative(')]
probe=adapter[adapter.index('u8 Blue_ReadLaunchMetadata('):]
PRE=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t FSIZE_t;
typedef unsigned UINT;
typedef char TCHAR;
typedef int FRESULT;
typedef struct {unsigned position;int destination;} FIL;
#define FA_READ 1
#define FA_WRITE 2
#define FA_CREATE_ALWAYS 4
#define FR_OK 0
#define MAX_pReadCache_size 0x20000
#define PSRAM_CAPACITY_BYTES 0x4000000
#define KERNEL_PLUGIN_ROM_LIMIT 0x2000000
#define SAVE_info_offset 0x780000
#define LX16_newomega_top_bin_size 0x716A0
#define LX16_FW_crc32 0x22DD74FA
static FIL gfile;
static unsigned char pReadCache[MAX_pReadCache_size];
static unsigned char *input;
static uint64_t input_size;
static unsigned input_capacity;
static unsigned char output[4096];
static unsigned output_size,reads,writes,closes;
static int fail_open,fail_close,fail_read,fail_write,short_read,short_write;
static void UIAudio_StopForSharedBufferUse(void) {}
void Launcher_ResetThumbCache(void) {}
#define f_size(file) ((file)->destination ? output_size : input_size)
static int f_open(FIL *file,const char *name,int mode) {
    (void)name;
    if(fail_open)return 1;
    file->position=0;file->destination=!!(mode&FA_WRITE);
    if(file->destination)output_size=0;
    return 0;
}
static int f_lseek(FIL *file,unsigned position) {file->position=position;return 0;}
static int f_read(FIL *file,void *buffer,unsigned bytes,unsigned *received) {
    assert(!file->destination);reads++;*received=0;
    if(fail_read)return 1;
    if(bytes>input_capacity-file->position)return 1;
    unsigned count=bytes-(short_read?1:0);
    memcpy(buffer,input+file->position,count);file->position+=count;*received=count;return 0;
}
static int f_write(FIL *file,const void *buffer,unsigned bytes,unsigned *written) {
    assert(file->destination);writes++;*written=0;
    if(fail_write)return 1;
    unsigned count=bytes-(short_write?1:0);assert(count<=sizeof output-file->position);
    memcpy(output+file->position,buffer,count);file->position+=count;output_size=file->position;*written=count;return 0;
}
int f_sync(FIL *file) {(void)file;return 0;}
static int f_close(FIL *file) {(void)file;closes++;return fail_close;}
static int f_unlink(const char *name) {(void)name;output_size=0;return 0;}
'''
POST=r'''
static void reset(uint64_t size) {
    input_size=size;output_size=reads=writes=closes=0;
    fail_open=fail_close=fail_read=fail_write=short_read=short_write=0;
}
int main(int argc,char **argv) {
    assert(argc==2);input_capacity=0x2069DC;input=calloc(1,input_capacity);assert(input);
    memset(input,0x5A,512);reset(512);
    assert(Copy_file("input","output") && output_size==512 && !memcmp(input,output,512));
    reset(512);fail_close=1;assert(!Copy_file("input","output") && !output_size && closes==2);
    reset(512);short_write=1;assert(!Copy_file("input","output") && !output_size);
    reset(512);short_read=1;assert(!Copy_file("input","output") && !output_size);
    reset(512);fail_write=1;assert(!Copy_file("input","output") && !output_size);
    reset(512);fail_read=1;assert(!Copy_file("input","output") && !output_size);
    reset(UINT64_C(0x100000200));assert(!Copy_file("input","output") && !reads && !writes);

    u32 bytes;u16 flag;u8 code[4];
    reset(1);assert(Blue_ReadLaunchMetadata("input",1,&bytes,code,&flag));
    assert(bytes==1 && !flag && !memcmp(code,"FFFF",4) && !reads && closes==1);
    reset(1);assert(!Blue_ReadLaunchMetadata("input",0,&bytes,code,&flag) && !reads && closes==1);
    reset(0);assert(!Blue_ReadLaunchMetadata("input",1,&bytes,code,&flag));
    reset(UINT64_C(0x100000001));assert(!Blue_ReadLaunchMetadata("input",1,&bytes,code,&flag) && !reads);
    memcpy(input+0xAC,"TEST",4);memcpy(input+0xB0,"5G",2);
    reset(0x4000000);assert(Blue_ReadLaunchMetadata("input",0,&bytes,code,&flag));
    assert(bytes==0x4000000 && flag==1 && !memcmp(code,"TEST",4));
    reset(0x2000000);assert(Blue_ReadLaunchMetadata("input",0,&bytes,code,&flag) && !flag);
    reset(0xC0);short_read=1;assert(!Blue_ReadLaunchMetadata("input",0,&bytes,code,&flag));
    reset(0xC0);fail_close=1;assert(!Blue_ReadLaunchMetadata("input",0,&bytes,code,&flag));

    FILE *resource=fopen(argv[1],"rb");assert(resource);
    assert(fread(input+0x100000,1,1075676,resource)==1075676);assert(!fclose(resource));
    input[0xB2]=0x96;reset(input_capacity);assert(Blue_ValidateKernelCandidate("input"));
    input[0x195004]^=1;reset(input_capacity);assert(!Blue_ValidateKernelCandidate("input"));input[0x195004]^=1;
    input[0xB2]=0;reset(input_capacity);assert(!Blue_ValidateKernelCandidate("input"));input[0xB2]=0x96;
    reset(input_capacity-1);assert(!Blue_ValidateKernelCandidate("input"));
    reset(UINT64_C(0x100000001));assert(!Blue_ValidateKernelCandidate("input"));
    reset(input_capacity);short_read=1;assert(!Blue_ValidateKernelCandidate("input"));
    reset(input_capacity);fail_close=1;assert(!Blue_ValidateKernelCandidate("input"));
    free(input);
    puts("BLUE launch/update: tiny plugin input, native 64 MiB flags, exact copies/close failures and firmware identity passed.");
}
'''
with tempfile.TemporaryDirectory() as folder:
    c=Path(folder)/'test.c';binary=Path(folder)/'test'
    c.write_text(PRE+copy+candidate+probe+POST)
    subprocess.run([os.environ.get('CC','cc'),'-std=gnu11','-O1','-Wall','-Wextra','-Werror',
                    '-fsanitize=address,undefined','-fno-pie','-no-pie',str(c),'-o',str(binary)],check=True)
    subprocess.run([str(binary),str(ROOT/'targets/blue/resources/image.bin')],check=True)
if len(sys.argv)==1:
    stage=main[main.index('u32 Stage_kernel_update('):main.index('//---------------------------------------------------------------------------------',main.index('u32 Stage_kernel_update('))]
    assert '"/ezbluekernel.bin"' in stage and '"/ezkernelnew.bin"' not in stage
    assert stage.index('Blue_ValidateKernelCandidate(tmp_path)')<stage.index('f_unlink(dst_path)')
