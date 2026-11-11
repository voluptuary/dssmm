
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
int f_lseek(FIL *file,unsigned position) {file->position=position;return 0;}
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

#include "omega-copy.impl.h"

static void reset(uint64_t size) {
    input_size=size;output_size=reads=writes=closes=0;
    fail_open=fail_close=fail_read=fail_write=short_read=short_write=0;
}
int main(void) {
    input_capacity=0x2069DC;input=calloc(1,input_capacity);assert(input);
    memset(input,0x5A,512);reset(512);
    assert(Copy_file("input","output") && output_size==512 && !memcmp(input,output,512));
    reset(512);fail_close=1;assert(!Copy_file("input","output") && !output_size && closes==2);
    reset(512);short_write=1;assert(!Copy_file("input","output") && !output_size);
    reset(512);short_read=1;assert(!Copy_file("input","output") && !output_size);
    reset(512);fail_write=1;assert(!Copy_file("input","output") && !output_size);
    reset(512);fail_read=1;assert(!Copy_file("input","output") && !output_size);
    reset(UINT64_C(0x100000200));assert(!Copy_file("input","output") && !reads && !writes);

    free(input);puts("Omega exact copy/close/full-width size checks passed.");
}
