#!/usr/bin/env python3
"""Run production save gates and BLUE RTS functions with file/DDR fault injection."""
from pathlib import Path
import os
import subprocess
import tempfile
import sys
ROOT=Path(__file__).resolve().parents[1]
def read(path):
    if len(sys.argv)>1:
        return subprocess.check_output(['git','show',sys.argv[1]+':ezfblue/targets/blue/source/'+path]).decode('latin1')
    return (ROOT/'targets/blue/source'/path).read_bytes().decode('latin1')
def function(text,name):
    start=text.index('u32 '+name+'(') if name!='Process_savefile' else text.index('u8 '+name+'(')
    end=text.index('\n}',start)+2
    return text[start:end]
main=read('ezbluekernel.c');patch=read('GBApatch.c')
code=function(main,'LoadRTSfile')+function(main,'Process_savefile')+function(patch,'Check_RTS')
PRE=r'''
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "blue_settings_policy.h"
typedef uint8_t u8;typedef uint16_t u16;typedef uint32_t u32;typedef uint8_t BYTE;
typedef uint64_t FSIZE_t;typedef unsigned UINT;typedef char TCHAR;typedef int FRESULT;
typedef struct {unsigned pos,writing;} FIL;
#define FR_OK 0
#define FR_NO_FILE 4
#define FR_EXIST 8
#define FA_READ 1
#define FA_WRITE 2
#define FA_CREATE_NEW 4
#define FA_OPEN_EXISTING 0
#define FA_OPEN_ALWAYS 8
#define SAVER_FOLDER "/SYSTEM/SAVER"
#define MAX_pReadCache_size 0x20000
#define DMA_COPY_MODE 1
static FIL gfile;
static unsigned char pReadCache[MAX_pReadCache_size],disk[0x70000],ddr[0x70000+16];
static unsigned short SAV_info_buffer[256];
static unsigned FAT_table_buffer[256];
static struct {unsigned csize;} EZcardFs;
static int gl_toggle_backup,gl_make_RTS,gl_make_sav;
static uint64_t disk_size;
static unsigned pending,mutations,opens,closes,reads,writes,seeks,page,copies,sram_writes;
static int missing,bad_open,bad_read,short_read,bad_close,bad_write,short_write,bad_seek,bad_fat;
static u16 Read_sav_info(unsigned index){(void)index;return pending;}
static int f_chdir(const char *name){(void)name;mutations++;return 0;}
static int f_mkdir(const char *name){(void)name;return 0;}
static int f_open(FIL *file,const char *name,unsigned mode){
    (void)name;opens++;file->pos=0;file->writing=!!(mode&FA_WRITE);
    if(bad_open)return 1;
    if(file->writing){assert(missing && (mode&FA_CREATE_NEW));missing=0;disk_size=0;return 0;}
    return missing?FR_NO_FILE:0;
}
#define f_size(file) (disk_size)
static int f_read(FIL *file,void *data,unsigned bytes,unsigned *got){
    reads++;*got=0;if((int)reads==bad_read)return 1;
    assert(file->pos+bytes<=sizeof disk);assert(file->pos+bytes<=disk_size);
    if((int)reads==short_read)bytes--;
    memcpy(data,disk+file->pos,bytes);file->pos+=bytes;*got=bytes;return 0;
}
static int f_write(FIL *file,const void *data,unsigned bytes,unsigned *got){
    writes++;*got=0;if((int)writes==bad_write)return 1;
    if((int)writes==short_write)bytes--;
    assert(file->writing && file->pos+bytes<=sizeof disk);
    memcpy(disk+file->pos,data,bytes);file->pos+=bytes;disk_size=file->pos;*got=bytes;return 0;
}
static int f_lseek(FIL *file,unsigned pos){seeks++;if((int)seeks==bad_seek)return 1;file->pos=pos;return 0;}
static int f_close(FIL *file){(void)file;closes++;return (int)closes==bad_close;}
static int f_unlink(const char *name){(void)name;missing=1;return 0;}
static void SetPSRampage(unsigned value){assert(value==0 || value==0x6000);page=value;}
#define PSRAMBase_S98 ((uintptr_t)ddr-0x20000)
static void dmaCopy(const void *source,uintptr_t target,unsigned count){
    assert(page==0x6000 && target>=(uintptr_t)ddr && target+count<=(uintptr_t)ddr+0x70000);
    copies++;memcpy((void *)target,source,count);
}
/* Retained in the fixture so the pre-fix SRAM-based loader compiles too. */
static void SetRampage(unsigned value){(void)value;}
#define SRAMSaver 0
static void WriteSram(unsigned addr,const void *data,unsigned bytes){(void)addr;(void)data;(void)bytes;sram_writes++;}
static int FileTxn_Recover(void){return 1;}
static int SaveFileOpenMeansMissing(FRESULT value){return value==FR_NO_FILE;}
static void Backup_savefile(const char *name){(void)name;}
static void ShowbootProgress(int message){(void)message;}
static unsigned Get_savefilesize(unsigned mode){(void)mode;return 0x10000;}
static unsigned SavefileWrite(const char *name,unsigned bytes){(void)name;(void)bytes;mutations++;return 1;}
static unsigned Loadsavefile(const char *name){(void)name;mutations++;return 1;}
static unsigned Save_sav_info(const void *data,unsigned bytes){(void)data;(void)bytes;mutations++;return 1;}
static void Bank_Switching(unsigned value){(void)value;}
static unsigned Check_game_RTS_FAT(const char *name,unsigned mode){(void)name;assert(mode==3);return bad_fat?0xffffffff:0;}
'''
POST=r'''
static void reset(void){
    memset(disk,0x5A,sizeof disk);memcpy(disk+0x62ff0,"EZ-OmegaRTCFILE.",16);
    memset(ddr,0xCC,sizeof ddr);disk_size=sizeof disk;
    pending=mutations=opens=closes=reads=writes=seeks=page=copies=sram_writes=0;
    missing=bad_open=bad_read=short_read=bad_close=bad_write=short_write=bad_seek=bad_fat=0;
}
int main(void){
    reset();pending=0x11;
    assert(Process_savefile(0,"game.gba",0x10000,0)==5 && !mutations && !opens);
    reset();assert(Process_savefile(0,"game.gba",0x10000,0)==0 && mutations && opens);
    reset();assert(LoadRTSfile("state.rts") && copies==7 && !page && !sram_writes);
    assert(!memcmp(ddr,disk,sizeof disk));for(unsigned i=0x70000;i<sizeof ddr;i++)assert(ddr[i]==0xCC);
    uint64_t invalid[]={0,1,0x6ffff,0x70001,UINT64_C(0x100070000)};
    for(unsigned i=0;i<sizeof invalid/sizeof invalid[0];i++){
        reset();disk_size=invalid[i];assert(!LoadRTSfile("state.rts") && !copies && closes==1);
    }
    reset();disk[0x62ff0]^=1;assert(!LoadRTSfile("state.rts") && !copies);
    reset();memset(disk,0,sizeof disk);assert(!LoadRTSfile("state.rts") && !copies);
    reset();memset(disk,0xff,sizeof disk);assert(LoadRTSfile("new.rts") && copies==7);
    for(unsigned fault=1;fault<=8;fault++){
        reset();bad_read=fault;assert(!LoadRTSfile("state.rts") && !page);
        reset();short_read=fault;assert(!LoadRTSfile("state.rts") && !page);
    }
    for(unsigned fault=1;fault<=2;fault++){
        reset();bad_seek=fault;assert(!LoadRTSfile("state.rts") && !copies);
    }
    reset();bad_close=1;assert(!LoadRTSfile("state.rts") && !page);
    reset();assert(Check_RTS("game.gba")==0x70000 && !writes);
    reset();missing=1;assert(Check_RTS("game.gba")==0x70000 && writes==4 && copies==7 && disk_size==0x70000);
    for(unsigned fault=1;fault<=4;fault++){
        reset();missing=1;bad_write=fault;assert(!Check_RTS("game.gba") && missing && !copies);
        reset();missing=1;short_write=fault;assert(!Check_RTS("game.gba") && missing && !copies);
    }
    reset();bad_open=1;assert(!Check_RTS("game.gba") && !writes && disk_size==0x70000);
    reset();disk_size=1;assert(!Check_RTS("game.gba") && !writes && !copies);
    reset();bad_fat=1;assert(!Check_RTS("game.gba"));
    reset();bad_close=1;assert(!Check_RTS("game.gba") && !copies);
    reset();missing=1;bad_close=1;assert(!Check_RTS("game.gba") && missing && !copies);
    assert(BlueBootPreference(0x4452,0)==2 && BlueBootPreference(0xffff,0x101)==1);
    assert(BlueBootPreference(0x4453,0x201)==0 && BlueBootPreference(0xffff,0xffff)==0);
    puts("BLUE save protection/RTS: pending FRAM preserved; exact sizes/signature, DDR banks, I/O faults, safe creation and preference migration passed.");
}
'''
with tempfile.TemporaryDirectory() as folder:
    c=Path(folder)/'test.c';binary=Path(folder)/'test';c.write_text(PRE+code+POST)
    subprocess.run([os.environ.get('CC','cc'),'-std=gnu11','-O1','-Wall','-Wextra','-Werror','-Wno-unused-function',
                    '-fsanitize=address,undefined','-fno-pie','-no-pie','-I',str(ROOT/'targets/blue/source'),str(c),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
if len(sys.argv)==1:
    fram=main[main.index('u8 FRAM_save_op('):main.index('static u32 Launcher_CanDeleteNorTitle(')]
    assert fram.index('Read_sav_info(0)')<fram.index('Launcher_DrawPopupPanel(')
    assert 'pNorFS[0].rompage != First_game_address / 0x20000' in fram
    assert 'Check_FW_update();' not in main
    assert 'SET_info_buffer[assress_ModeB_INIT] = gl_ModeB_init;' in main

    nor=main[main.index('void Boot_NOR_game('):main.index('u8 FRAM_save_op(')]
    assert nor.index('.have_RTS')<nor.index('Process_savefile(')
    assert 'Turn off save states for NOR add-ons.' in main

    startup=main[main.index('int main(void)'):main.index('refind_file:')]
    assert 'Save_NOR_FAT_info(' not in startup and 'Save_NOR_info_with_art_path(' not in startup
