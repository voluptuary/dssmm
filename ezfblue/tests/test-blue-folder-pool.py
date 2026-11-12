#!/usr/bin/env python3
"""Protect PSRAM against CPU writes while exercising actual folder staging/sorting."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
s=(ROOT/'targets/blue/source/ezbluekernel.c').read_text()
helpers=s[s.index('static u32 Launcher_IsCP936LeadByte('):s.index('static void Launcher_StoreFileRecord(')]
store=s[s.index('static void Launcher_StoreFolderRecord('):s.index('static char (*Launcher_FavouritesBuffer',s.index('static void Launcher_StoreFolderRecord('))]
sort=s[s.index('void Sort_folder('):s.index('void Sort_file(')]
header=(ROOT/'targets/blue/source/ezkernel.h').read_text()
folder=header[header.index('typedef struct FM_Folder_SECT'):header.index('typedef struct FM_FILE_SECT')]
PRE=r'''
#define _GNU_SOURCE
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
typedef uint8_t u8;
typedef uint32_t u32;
typedef char TCHAR;
#define FM_ENTRY_ALIAS_DISPLAY 1
#define MAX_folder 256
'''
MOCK=r'''
#define pFolder ((FM_Folder_FS *)0x08FE0000)
static _Alignas(4) FM_Folder_FS launcher_folder_sort_temp, launcher_folder_sort_probe;
static unsigned copies;
static void dmaCopy(const void *src,void *dst,u32 bytes) {
    assert(!((uintptr_t)src&1) && !((uintptr_t)dst&1) && !(bytes&1));
    uintptr_t address=(uintptr_t)dst;
    int psram=address>=0x08FE0000 && address<0x09000000;
    uintptr_t input=(uintptr_t)src;
    assert(!(psram && input>=0x08FE0000 && input<0x09000000));
    if(psram) {
        assert(address+bytes<=0x08FF0000);
        assert(!mprotect((void *)0x08FE0000,0x20000,PROT_READ|PROT_WRITE));
    }
    memmove(dst,src,bytes);copies++;
    if(psram)assert(!mprotect((void *)0x08FE0000,0x20000,PROT_READ));
}
'''
POST=r'''
static int compare(const void *a,const void *b) {
    const FM_Folder_FS *left=a,*right=b;
    int order=strcmp(left->filename,right->filename);
    return order ? order : strcmp(left->altname,right->altname);
}
int main(void) {
    void *mapped=mmap((void *)0x08FE0000,0x20000,PROT_READ|PROT_WRITE,
                      MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0);
    assert(mapped==(void *)0x08FE0000);
    FM_Folder_FS expected[256];
    unsigned counts[]={0,1,2,31,96,255,256};
    for(unsigned test=0;test<sizeof counts/sizeof counts[0];test++) {
        unsigned count=counts[test];
        assert(!mprotect(mapped,0x20000,PROT_READ|PROT_WRITE));memset(mapped,0xA5,0x20000);
        assert(!mprotect(mapped,0x20000,PROT_READ));copies=0;
        for(unsigned i=0;i<count;i++) {
            char name[130],alias[20];
            snprintf(name,sizeof name,"folder%03u",count-i);
            snprintf(alias,sizeof alias,"F%06u",i);
            Launcher_StoreFolderRecord(i,name,alias);
        }
        unsigned saved=copies;
        Launcher_StoreFolderRecord(256,"outside","OUT");assert(copies==saved);
        memcpy(expected,pFolder,count*sizeof(FM_Folder_FS));
        qsort(expected,count,sizeof(FM_Folder_FS),compare);
        Sort_folder(count);
        assert(!memcmp(expected,pFolder,count*sizeof(FM_Folder_FS)));
        const unsigned char *bytes=mapped;
        for(unsigned i=count*sizeof(FM_Folder_FS);i<0x20000;i++) assert(bytes[i]==0xA5);
    }
    char long_name[200];memset(long_name,'X',199);long_name[199]=0;
    Launcher_StoreFolderRecord(0,long_name,"LONGNA~1");
    assert(pFolder[0].flags&FM_ENTRY_ALIAS_DISPLAY);
    assert(pFolder[0].filename[99]==0 && !strcmp(pFolder[0].altname,"LONGNA~1"));
    assert(!munmap(mapped,0x20000));
    puts("BLUE folder pool: 256 folders, clipped identities, reference sort and DMA-only PSRAM writes passed; file/index pool guards intact.");
}
'''
with tempfile.TemporaryDirectory() as directory:
    c=Path(directory)/'test.c';binary=Path(directory)/'test'
    c.write_text(PRE+folder+MOCK+helpers+store+sort+POST)
    subprocess.run([os.environ.get('CC','cc'),'-std=gnu11','-O1','-Wall','-Wextra','-Werror',
                    '-fsanitize=address,undefined','-fno-pie','-no-pie',str(c),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
