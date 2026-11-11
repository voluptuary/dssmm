#include <gba_interrupt.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <gba_base.h>
#include <gba_dma.h>
#include <string.h>
#include <gba_systemcalls.h>

#include "NORflash_OP.h"
#include "Ezcard_OP.h"
#include "ezkernel.h"
#include "draw.h"
#include "lang.h"
#include "GBApatch.h"
#define DEBUG

extern void delay(u32 R0);
extern void IWRAM_CODE PatchInternal(u32* Data,int iSize,u32 offset);

extern FM_NOR_FS pNorFS[MAX_NOR]EWRAM_BSS;
extern u8 pReadCache [MAX_pReadCache_size]EWRAM_BSS;
extern u32 gl_currentpage;
extern u32 gl_norOffset;
extern FIL gfile;
extern u32 game_total_NOR;
extern u32 iTrimSize;

//---------------------------------------------------------------
void Chip_Reset()
{
    *((vu16 *)(FlashBase_S98)) = 0xF0 ;
}
//---------------------------------------------------------------
void Block_Erase(u32 blockAdd) //0x20000 BYTE pre block
{
    vu16 page,v1,v2;
    u32 Address;
    u32 loop;
    page=gl_currentpage;
    Address=blockAdd;
    while(Address>=0x800000) {
        Address-=0x800000;
        page+=0x1000;
    }
    //u8 ramdata = *((vu8 *)(0xE000000+1)) ;
    SetRompage(page);
    //u16 norid = Read_S98NOR_ID();
    Chip_Reset();
    v1=0;
    v2=1;
    if((blockAdd==0) || (blockAdd==0x3FE0000)) {
        //Address=blockAdd;
        for(loop=0; loop<0x20000; loop+=0x8000) {
            *((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
            *((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
            *((vu16 *)(FlashBase_S98+0x555*2)) = 0x80 ;
            *((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
            *((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
            *((vu16 *)(FlashBase_S98+Address+loop)) = 0x30 ;
            do {
                v1 = *((vu16 *)(FlashBase_S98+Address+loop)) ;
                v2 = *((vu16 *)(FlashBase_S98+Address+loop)) ;
            }
            while(v1!=v2);
        }
    }
    else {
        loop=Address;
        *((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
        *((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
        *((vu16 *)(FlashBase_S98+0x555*2)) = 0x80 ;
        *((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
        *((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
        *((vu16 *)(FlashBase_S98+loop)) = 0x30 ;
        do {
            v1 = *((vu16 *)(FlashBase_S98+loop)) ;
            v2 = *((vu16 *)(FlashBase_S98+loop)) ;
        }
        while(v1!=v2);
    }
    SetRompage(gl_currentpage);
}
//-----------------------------------------------------------
void Chip_Erase()
{
    char msg[128];
    u32 count=0;
    vu16 v1,v2=0 ;
    REG_IME = 0 ;
    *((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
    *((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
    *((vu16 *)(FlashBase_S98+0x555*2)) = 0x80 ;
    *((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
    *((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
    *((vu16 *)(FlashBase_S98+0x555*2)) = 0x10 ;
	DrawPic((u16*)gImage_MENU, 36, 25, 168, 110, 1, 0, 1);//show menu pic
    do {
        VBlankIntrWait();
        ShowTime(NOR_list,0);
        itoa(count,msg,2);
		DrawHZText12("Erasing NOR, please wait.",0,45,65,gl_color_text,1);
        DrawHZText12(msg,0,75,80,gl_color_text,1);
        count++;
        VBlankIntrWait();
        v1 = *((vu16 *)(FlashBase_S98)) ;
        v2 = *((vu16 *)(FlashBase_S98)) ;
    }
    while(v1!=v2);
    REG_IME = 1 ;
}

//-----------------------------------------------------------
void FormatNor()
{
    char msg[128];
    sprintf(msg,"%s",gl_formatnor_info1);
    DrawHZText12(gl_formatnor_info1,0,80,80,gl_color_text,1);
    DrawHZText12(gl_formatnor_info2,0,52,95,gl_color_text,1);
    while(1) {
        delay(500);
        scanKeys();
        u16 keys = keysDown();
        if (keys & KEY_A) {
            Chip_Erase();
			VBlankIntrWait();
            memset(pNorFS,00,sizeof(FM_NOR_FS)*MAX_NOR);
            return;
        }
        else if(keys & KEY_B) {
            return;
        }
    }
}
//---------------------------------------------------------------
void WriteFlash(u32 address,u8 *buffer,u32 size)
{
    vu16 page,v1,v2;
    register u32 loopwrite ;
    vu16* buf = (vu16*)buffer ;
    page=gl_currentpage;
    while(address>=0x800000) {
        address-=0x800000;
        page+=0x1000;
    }
    SetRompage(page);
    Chip_Reset();
    v1=0;
    v2=1;
    for(loopwrite=0; loopwrite<(size/2); loopwrite++) {
        *((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
        *((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
        *((vu16 *)(FlashBase_S98+0x555*2)) = 0xA0 ;
        *((vu16 *)(FlashBase_S98+address+loopwrite*2)) = buf[loopwrite];
        do {
            v1 = *((vu16 *)(FlashBase_S98+address+loopwrite*2)) ;
            v2 = *((vu16 *)(FlashBase_S98+address+loopwrite*2)) ;
        }
        while(v1!=v2);
    }
    SetRompage(gl_currentpage);
}
//---------------------------------------------------------------
void IWRAM_CODE WriteFlash_with32word(u32 address,u8 *buffer,u32 size)
{
    vu16 page,v1,v2;
    register u32 loopwrite ;
    vu16* buf = (vu16*)buffer ;
    u32 i;
    page=gl_currentpage;
    while(address>=0x800000) {
        address-=0x800000;
        page+=0x1000;
    }
    SetRompage(page);
    Chip_Reset();
    v1=0;
    v2=1;
    for(loopwrite=0; loopwrite<(size/32); loopwrite++) {
        *((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
        *((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
        *((vu16 *)(FlashBase_S98+address+loopwrite*32)) = 0x25;
        *((vu16 *)(FlashBase_S98+address+loopwrite*32)) = 15;
        for(i=0; i<=15; i++) {
            *((vu16 *)(FlashBase_S98+address+loopwrite*32 +2*i )) = buf[loopwrite*16+i];
        }
        *((vu16 *)(FlashBase_S98+address+loopwrite*32)) = 0x29;
        do {
            v1 = *((vu16 *)(FlashBase_S98+address+loopwrite*32+0xF*2)) ;
            v2 = *((vu16 *)(FlashBase_S98+address+loopwrite*32+0xF*2)) ;
        }
        while(v1!=v2);
    }
    SetRompage(gl_currentpage);
}
//-----------------------------------------------------------
#define NOR_MENU_WIDTH     168
#define NOR_PROGRESS_X     72
#define NOR_PROGRESS_Y     104
#define NOR_PROGRESS_W     96
#define NOR_PROGRESS_H     8

static void NOR_DrawWriteProgress(u32 completed, u32 total, u32 *last_percent)
{
	u32 percent = completed * 100 / total;
	u32 filled = completed * NOR_PROGRESS_W / total;
	u32 row;
	char text[12];
	const u16 *menu = (const u16*)gImage_MENU;
	if(percent == *last_percent)
		return;
	*last_percent = percent;
	Launcher_UpdateNORWriteClock(0);
	for(row = 57; row < 69; row++)
		DrawPic((u16*)(menu + row * NOR_MENU_WIDTH), 36, 25 + row,
			NOR_MENU_WIDTH, 1, 0, 0, 1);
	snprintf(text, sizeof(text), "%lu%%", (unsigned long)percent);
	DrawHZText12(text, 0, (240 - strlen(text) * 6) / 2, 82, gl_color_text, 1);
	Clear(NOR_PROGRESS_X, NOR_PROGRESS_Y, NOR_PROGRESS_W, NOR_PROGRESS_H,
		gl_color_selectBG_sd, 1);
	if(filled)
		Clear(NOR_PROGRESS_X, NOR_PROGRESS_Y, filled, NOR_PROGRESS_H,
			gl_color_text, 1);
}

u32 Loadfile2NOR(TCHAR *filename, u32 NORaddress,u32 have_patch)
{
    FSIZE_t file_size;
    UINT got;
    u32 filesize, fileneedsize, blocknum, read_size;
    u32 add_patch = 0;
    FM_NOR_FS tmpNorFS;
    u32 last_percent = 101;
    u32 progress_blocks;
    if(!filename || strlen(filename) >= sizeof(tmpNorFS.filename))
        return NOR_WRITE_FILE_ERROR;
    if(game_total_NOR >= MAX_NOR || NORaddress >= 0x4000000 ||
        (NORaddress & 0x1FFFF)) return 2;
    if(Read_S98NOR_ID() != 0x223D) return 1;
    if(f_open(&gfile, filename, FA_READ) != FR_OK) return NOR_WRITE_FILE_ERROR;
    file_size = f_size(&gfile);
    if(file_size < 0xB0) goto file_failed;
    if(file_size > 0x4000000) goto full;
    filesize = (u32)file_size;
    fileneedsize = (filesize + 0x1FFFF) & ~0x1FFFFUL;
    if(have_patch && iTrimSize >= fileneedsize)
    {
        fileneedsize += 0x20000;
        add_patch = 1;
    }
    if(fileneedsize > 0x4000000 - NORaddress) goto full;
    memset(&tmpNorFS, 0, sizeof(tmpNorFS));
    if(f_lseek(&gfile, 0xA0) != FR_OK ||
        f_read(&gfile, tmpNorFS.gamename, sizeof(tmpNorFS.gamename), &got) != FR_OK ||
        got != sizeof(tmpNorFS.gamename)) goto file_failed;
    tmpNorFS.rompage = NORaddress >> 17;
    tmpNorFS.filesize = fileneedsize;
    tmpNorFS.have_patch = have_patch;
    tmpNorFS.have_RTS = gl_rts_on;
    strcpy(tmpNorFS.filename, filename);
        ////////////////// erase all BBP
        *((vu16 *)(FlashBase_S98)) = 0xF0 ;
        *((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
        *((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
        *((vu16 *)(FlashBase_S98+0x555*2)) = 0xC0 ;
        *((vu16 *)(FlashBase_S98+0x000*2)) = 0x80 ;
        *((vu16 *)(FlashBase_S98+0x000*2)) = 0x30 ;
        {
            int polling_counter = 0x15000;
            do {
                (void)*((vu16 *)(FlashBase_S98+ 0x5C0000));
                polling_counter--;
            }
            while (polling_counter);
        }
        *((vu16 *)(FlashBase_S98+0x000*2)) = 0x90 ;
        *((vu16 *)(FlashBase_S98+0x000*2)) = 0x00 ;
        /////////////////

    progress_blocks = fileneedsize / 0x20000;
    Launcher_DrawNORWriteBackground();
    DrawPic((u16*)gImage_MENU,36,25,NOR_MENU_WIDTH,110,0,0,1);
    {
        u16 length = DrawText12VisibleLength(gl_writing);
        if(length > 26) length = 26;
        DrawHZText12(gl_writing,length,(240-length*6)/2,60,gl_color_text,1);
    }
    for(blocknum=0; blocknum<filesize; blocknum+=0x20000)
    {
        NOR_DrawWriteProgress(blocknum / 0x20000, progress_blocks, &last_percent);
        read_size = filesize - blocknum;
        if(read_size > 0x20000) read_size = 0x20000;
        memset(pReadCache, 0xFF, 0x20000);
        if(f_lseek(&gfile, blocknum) != FR_OK ||
            f_read(&gfile, pReadCache, read_size, &got) != FR_OK || got != read_size)
            goto file_failed;
        /* Never erase/program a block whose SD contents were not read fully. */
        Block_Erase(blocknum+NORaddress);
        if(have_patch)
        {
            if(gl_reset_on || gl_rts_on || gl_sleep_on || gl_cheat_on)
            {
                PatchInternal((u32*)pReadCache,0x20000,blocknum);
                GBApatch_NOR((u32*)pReadCache,0x20000,blocknum);
            }
        }
        else GBApatch_Cleanrom_NOR((u32*)pReadCache,blocknum);
        WriteFlash_with32word(blocknum+NORaddress,pReadCache,0x20000);
    }
    if(f_close(&gfile) != FR_OK) return NOR_WRITE_FILE_ERROR;
    if(have_patch && add_patch)
    {
        NOR_DrawWriteProgress(blocknum / 0x20000, progress_blocks, &last_percent);
        Block_Erase(blocknum+NORaddress);
        GBApatch_NOR((u32*)pReadCache,0x20000,blocknum);
        WriteFlash_with32word(blocknum+NORaddress,pReadCache,0x20000);
    }
    /* Backend flash functions are void: this checks SD input, not flash success. */
    dmaCopy(&tmpNorFS,&pNorFS[game_total_NOR],sizeof(tmpNorFS));
    Save_NOR_info((u16*)pNorFS,sizeof(FM_NOR_FS)*MAX_NOR);
    NOR_DrawWriteProgress(progress_blocks, progress_blocks, &last_percent);
    return 0;
full:
    (void)f_close(&gfile);
    return 2;
file_failed:
    (void)f_close(&gfile);
    return NOR_WRITE_FILE_ERROR;
}

//-----------------------------------------------------------
u32 GetFileListFromNor(void)
{
    u16 old_ime = REG_IME;
    REG_IME = 0;
    gl_norOffset = 0;
    u32 page=0,count=0;
    uintptr_t StartAddress = FlashBase_S98;
    char temp[50];
    vu16  Value;
    SetRompage(gl_currentpage);
    Value = *(vu16 *)(StartAddress + 0xbe);
    u16 x24 = *(vu16 *)(StartAddress + 0x6);
    //DEBUG_printf(" %x %x %x",StartAddress,Value,x24);
    while( ((Value&0xFF)==0xCE) || ((Value&0xFF)==0xCF)|| ((Value&0xFF)==0x00)|| (x24==0x51ae)) {
        if(count >= MAX_NOR) break;
        //DEBUG_printf(" %x %x %x",StartAddress,Value,x24);
        if(*(vu8 *)(StartAddress+0xb2) == 0x96) {
            memcpy(temp,(char*)(StartAddress+0xa0),0x10);
            //temp[12] = 0 ;
            //DEBUG_printf("  %s VS %s",temp ,pNorFS[count].gamename);
            if(pNorFS[count].filesize < 0x20000 ||
                (pNorFS[count].filesize & 0x1FFFF) ||
                pNorFS[count].filesize > 0x4000000 - gl_norOffset) break;
            if(memcmp(temp,pNorFS[count].gamename,0x10) ==0) { //if(!strcasecmp(temp, pNorFS[count].gamename))
                gl_norOffset += pNorFS[count].filesize;
                StartAddress += pNorFS[count].filesize;
                count ++ ;
            }
            else {
                break;
            }
        }
        else {
            break;
        }
        while(StartAddress >= FlashBase_S98_end) {
            page += 0x1000 ;
            if(page>0x7000) {
                SetRompage(gl_currentpage);
                REG_IME = old_ime;
                return count;
            }
            SetRompage(gl_currentpage+page);
            StartAddress -= 0x800000 ;
        }
        if(count >= MAX_NOR) {
            break;    //max
        }
        Value = *(vu16 *)(StartAddress + 0xbe);
        x24 = *(vu16 *)(StartAddress + 0x6);
    }
    SetRompage(gl_currentpage);
    REG_IME = old_ime;
    return count ;
}
//-----------------------------------------------------------
