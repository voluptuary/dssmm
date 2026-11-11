#!/usr/bin/env python3
"""Actual BLUE SD artwork records and allocation/catalog publication ordering."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
source=(ROOT/'targets/blue/source/blue_frontend_adapter.c').read_text()
art=source[source.index('static char blue_art_path'):source.index('void Read_NOR_info')]
commit=source[source.index('u8 Blue_CommitNorCatalog('):source.index('u8 Save_NOR_info(')]
header=(ROOT/'targets/blue/source/ezkernel.h').read_text()
catalog=header[header.index('typedef struct FM_NOR_FILE_SECT'):header.index('typedef struct FM_Folder_SECT')]
PRE=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "blue_nor_policy.h"
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
u8 NOR_FAT_buffer[960];
static u8 blue_planned_map[960], persisted_map[960];
static unsigned char disk[1024];
static unsigned disk_size, file_exists;
static char disk_name[48];
static int bad_read, bad_write, short_read, short_write, bad_close, bad_open;
static unsigned map_writes, catalog_writes, operation, fail_map, fail_catalog;
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
static u8 Save_NOR_FAT_info(void) {
    map_writes++;operation++;
    memcpy(persisted_map,NOR_FAT_buffer,960); return !fail_map;
}
static u8 Save_NORgame_info(u16 *buffer,u32 bytes) {
    assert(buffer==(u16 *)pNorFS && bytes==sizeof pNorFS);
    /* Every page of newly published live entries must already be reserved. */
    for(unsigned slot=0;slot<64;slot++) {
        if(!pNorFS[slot].filesize)continue;
        unsigned first=pNorFS[slot].rompage-64, pages=pNorFS[slot].filesize/0x20000;
        for(unsigned page=first;page<first+pages;page++) assert(persisted_map[page]);
    }
    catalog_writes++;operation++; return !fail_catalog;
}
'''
POST=r'''
static void reset(void) {
    memset(pNorFS,0,sizeof pNorFS);memset(NOR_FAT_buffer,0,960);memset(persisted_map,0,960);
    pNorFS[0].rompage=64;pNorFS[0].filesize=0x40000;strcpy(pNorFS[0].filename,"same.gba");
    bad_open=bad_close=bad_read=bad_write=short_read=short_write=0;
    file_exists=disk_size=map_writes=catalog_writes=operation=fail_map=fail_catalog=0;
}
static void valid_record(void) {
    Blue_SaveNorArtworkPath(0,"/GBA/Long Folder");
    assert(file_exists && !strcmp(disk_name,"/SYSTEM/NORART/0040.art"));
    assert(!strcmp(Read_NOR_art_path(0),"/GBA/Long Folder"));
}
static void rehash(void) {
    BlueNorArtworkRecord value;memcpy(&value,disk,sizeof value);
    value.checksum=Blue_Hash(&value,sizeof value-sizeof value.checksum);
    memcpy(disk,&value,sizeof value);
}
int main(void) {
    reset();valid_record();
    assert(!Read_NOR_art_path(64));
    pNorFS[0].filesize+=0x40000;assert(!Read_NOR_art_path(0));pNorFS[0].filesize-=0x40000;
    pNorFS[1]=pNorFS[0];pNorFS[1].rompage=66;assert(!Read_NOR_art_path(1));
    valid_record();disk[sizeof(BlueNorArtworkRecord)-1]^=1;assert(!Read_NOR_art_path(0));
    valid_record();disk_size++;assert(!Read_NOR_art_path(0));
    valid_record();BlueNorArtworkRecord value;memcpy(&value,disk,sizeof value);
    value.length=256;memcpy(disk,&value,sizeof value);rehash();assert(!Read_NOR_art_path(0));
    valid_record();disk[offsetof(BlueNorArtworkRecord,path)]='x';rehash();assert(!Read_NOR_art_path(0));
    valid_record();short_read=1;assert(!Read_NOR_art_path(0));short_read=0;
    valid_record();bad_read=1;assert(!Read_NOR_art_path(0));bad_read=0;
    valid_record();bad_close=1;assert(!Read_NOR_art_path(0));bad_close=0;
    bad_write=1;Blue_SaveNorArtworkPath(0,"/new");assert(!file_exists);bad_write=0;
    short_write=1;Blue_SaveNorArtworkPath(0,"/new");assert(!file_exists);short_write=0;
    bad_close=1;Blue_SaveNorArtworkPath(0,"/new");assert(!file_exists);bad_close=0;
    valid_record();Blue_SaveNorArtworkPath(0,"");assert(!file_exists);
    reset();assert(Blue_CommitNorCatalog());
    assert(map_writes==1 && catalog_writes==1 && operation==2);
    assert(persisted_map[0] && persisted_map[1] && !persisted_map[2]);
    memset(pNorFS,0,sizeof pNorFS);assert(Blue_CommitNorCatalog());
    assert(map_writes==2 && catalog_writes==2 && !persisted_map[0]);
    reset();fail_map=1;assert(!Blue_CommitNorCatalog() && map_writes==1 && !catalog_writes);
    reset();fail_catalog=1;assert(!Blue_CommitNorCatalog() && map_writes==1 && catalog_writes==1);
    assert(persisted_map[0] && persisted_map[1]);
    reset();pNorFS[1]=pNorFS[0];assert(!Blue_CommitNorCatalog() && !map_writes && !catalog_writes);
    reset();pNorFS[0].rompage=0xFFFF;assert(!Blue_CommitNorCatalog() && !map_writes && !catalog_writes);
    puts("BLUE artwork SD records and reserve/publish/release ordering passed (storage mocked).");
}
'''
with tempfile.TemporaryDirectory() as folder:
    c=Path(folder)/'test.c';binary=Path(folder)/'test'
    c.write_text('#include <stddef.h>\n'+PRE+catalog+MOCK+art+commit+POST)
    subprocess.run([os.environ.get('CC','cc'),'-std=gnu11','-O1','-Wall','-Wextra','-Werror',
                    '-fsanitize=address,undefined','-fno-pie','-no-pie',
                    '-I',str(ROOT/'targets/blue/source'),str(c),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
