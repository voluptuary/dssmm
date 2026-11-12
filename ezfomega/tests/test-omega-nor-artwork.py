#!/usr/bin/env python3
"""Actual Original OMEGA SD artwork records and failure handling."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
source=(ROOT/'source/nor_artwork_sd.c').read_text()
art=source[source.index('static char omega_art_path'):]
header=(ROOT/'source/ezkernel.h').read_text()
catalog=header[header.index('typedef struct FM_NOR_FILE_SECT'):header.index('typedef struct FM_Folder_SECT')]
PRE=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef unsigned UINT;
typedef int FRESULT;
typedef struct {unsigned pos;} FIL;
#define FR_OK 0
#define FA_READ 1
#define FA_WRITE 2
#define FA_CREATE_ALWAYS 4
#define MAX_NOR 64
#define NOR_FAT_SIZE 960
#define MAX_path_len 256
#define EWRAM_BSS
'''
MOCK=r'''
static FM_NOR_FS pNorFS[MAX_NOR];

static unsigned char disk[1024];
static unsigned disk_size, file_exists;
static char disk_name[48];
static int bad_read, bad_write, short_read, short_write, bad_close, bad_open;

static int f_mkdir(const char *name) {(void)name;return 0;}
static int f_stat(const char *name,void *info) {(void)name;(void)info;return 0;}
static int f_unlink(const char *name) {
    if(!strcmp(name,disk_name)) file_exists=0;
    return 0;
}
static unsigned f_size(FIL *file) {(void)file;return disk_size;}
static int f_open(FIL *file,const char *name,int mode) {
    if(bad_open)return 1;
    file->pos=0;
    if(mode&FA_WRITE) {strcpy(disk_name,name);file_exists=1;disk_size=0;return 0;}
    return file_exists && !strcmp(name,disk_name) ? 0 : 1;
}
static int f_read(FIL *file,void *buffer,unsigned bytes,unsigned *received) {
    if(bad_read){*received=0;return 1;}
    unsigned count=disk_size-file->pos;
    if(count>bytes)count=bytes;
    if(short_read && count)count--;
    memcpy(buffer,disk+file->pos,count);file->pos+=count;*received=count;return 0;
}
static int f_write(FIL *file,const void *buffer,unsigned bytes,unsigned *written) {
    if(bad_write){*written=0;return 1;}
    unsigned count=bytes-(short_write?1:0);assert(count<=sizeof disk);
    memcpy(disk,buffer,count);disk_size=count;file->pos=count;*written=count;return 0;
}
static int f_close(FIL *file) {(void)file;return bad_close;}
'''
POST=r'''
static void reset(void) {
    memset(pNorFS,0,sizeof pNorFS);
    pNorFS[0].rompage=64;pNorFS[0].filesize=0x40000;strcpy(pNorFS[0].filename,"same.gba");
    bad_open=bad_close=bad_read=bad_write=short_read=short_write=0;
    file_exists=disk_size=0;
}
static void valid_record(void) {
    Omega_SaveNorArtworkPath(0,"/GBA/Long Folder");
    assert(file_exists && !strcmp(disk_name,"/SYSTEM/NORART/0040.oma"));
    assert(!strcmp(Omega_ReadNorArtworkPath(0),"/GBA/Long Folder"));
}
static void rehash(void) {
    OmegaNorArtworkRecord value;memcpy(&value,disk,sizeof value);
    value.checksum=Omega_Hash(&value,sizeof value-sizeof value.checksum);
    memcpy(disk,&value,sizeof value);
}
int main(void) {
    reset();valid_record();
    assert(!Omega_ReadNorArtworkPath(64));
    pNorFS[0].filesize+=0x40000;assert(!Omega_ReadNorArtworkPath(0));pNorFS[0].filesize-=0x40000;
    pNorFS[1]=pNorFS[0];pNorFS[1].rompage=66;assert(!Omega_ReadNorArtworkPath(1));
    valid_record();disk[sizeof(OmegaNorArtworkRecord)-1]^=1;assert(!Omega_ReadNorArtworkPath(0));
    valid_record();disk_size++;assert(!Omega_ReadNorArtworkPath(0));
    valid_record();OmegaNorArtworkRecord value;memcpy(&value,disk,sizeof value);
    value.length=sizeof value.path;memcpy(disk,&value,sizeof value);rehash();assert(!Omega_ReadNorArtworkPath(0));
    valid_record();disk[offsetof(OmegaNorArtworkRecord,path)]='x';rehash();assert(!Omega_ReadNorArtworkPath(0));
    valid_record();short_read=1;assert(!Omega_ReadNorArtworkPath(0));short_read=0;
    valid_record();bad_read=1;assert(!Omega_ReadNorArtworkPath(0));bad_read=0;
    valid_record();bad_close=1;assert(!Omega_ReadNorArtworkPath(0));bad_close=0;
    bad_write=1;Omega_SaveNorArtworkPath(0,"/new");assert(!file_exists);bad_write=0;
    short_write=1;Omega_SaveNorArtworkPath(0,"/new");assert(!file_exists);short_write=0;
    bad_close=1;Omega_SaveNorArtworkPath(0,"/new");assert(!file_exists);bad_close=0;
    valid_record();Omega_SaveNorArtworkPath(0,"");assert(!file_exists);
    puts("Omega NOR SD artwork records: identity, corruption, exact I/O and failures passed (storage mocked).");
}
'''
with tempfile.TemporaryDirectory() as folder:
    c=Path(folder)/'test.c';binary=Path(folder)/'test'
    c.write_text('#include <stddef.h>\n'+PRE+catalog+MOCK+art+POST)
    subprocess.run([os.environ.get('CC','cc'),'-std=gnu11','-O1','-Wall','-Wextra','-Werror',
                    '-fsanitize=address,undefined','-fno-pie','-no-pie',
                    '-I',str(ROOT/'source'),str(c),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
