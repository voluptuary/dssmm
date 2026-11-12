#include <gba_interrupt.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include <gba_base.h>
#include <gba_dma.h>
#include <string.h>
#include <gba_systemcalls.h>

#include "ez_define.h"
#include "NORflash_OP.h"
#include "ezkernel.h"
#include "draw.h"
#include "lang.h"
#include "Ezcard_OP.h"
#include "GBApatch.h"
#include "blue_nor_policy.h"
extern void Blue_PrepareFlashAccess(void);
extern void Blue_DrawNorProgress(u32 completed, u32 total);
//#define DEBUG

extern FM_NOR_FS pNorFS[MAX_NOR]EWRAM_BSS;
extern u8 pReadCache [MAX_pReadCache_size]EWRAM_BSS;
extern u32 gl_currentpage;
extern u32 gl_norOffset;
extern FIL gfile;
extern u32 game_total_NOR;
extern u32 iTrimSize;

extern u8 NOR_FAT_buffer[NOR_FAT_SIZE]EWRAM_BSS;
//---------------------------------------------------------------
void IWRAM_CODE Chip_Reset()
{
    Blue_PrepareFlashAccess();
    u16 saved_ime = REG_IME;
    REG_IME = 0;

	SetRompage_MODE(0x8000,SYSTEM_MODE_OS);//select flash1
	*((vu16 *)(FlashBase_S98)) = 0xF0 ;
	SetRompage_MODE(0x0000,SYSTEM_MODE_OS);//select flash0
	*((vu16 *)(FlashBase_S98)) = 0xF0 ;
	

    REG_IME = saved_ime;
}
//---------------------------------------------------------------
void IWRAM_CODE BlueVendor_Block_Erase(u32 blockAdd) //0x20000 BYTE pre block
{
    Blue_PrepareFlashAccess();
    u16 saved_ime = REG_IME;
    REG_IME = 0;

	vu16 page,v1,v2;
	u32 Address;
	u32 loop;

	page=gl_currentpage;
	Address=blockAdd;
	while(Address>=0x800000)
	{
		Address-=0x800000;
		page+=0x1000;
	}
	//u8 ramdata = *((vu8 *)(0xE000000+1)) ;
	
	SetRompage_MODE(page,SYSTEM_MODE_OS);
	//u16 norid = Read_S98NOR_ID();

	v1=0;v2=1;
	if((blockAdd==0) || (blockAdd==0x3FE0000)   || (blockAdd==0x4000000) || (blockAdd==0x7FE0000) )
	{
		//Address=blockAdd;
		for(loop=0;loop<0x20000;loop+=0x8000)
		{
			*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
			*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
			*((vu16 *)(FlashBase_S98+0x555*2)) = 0x80 ;
			*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
			*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
			*((vu16 *)(FlashBase_S98+Address+loop)) = 0x30 ;
			do
			{
				v1 = *((vu16 *)(FlashBase_S98+Address+loop)) ;
				v2 = *((vu16 *)(FlashBase_S98+Address+loop)) ;
			}while(v1!=v2);
		}
	}
	else
	{
		loop=Address;

		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
		*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0x80 ;
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
		*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
		*((vu16 *)(FlashBase_S98+loop)) = 0x30 ;
		do
		{
			v1 = *((vu16 *)(FlashBase_S98+loop)) ;
			v2 = *((vu16 *)(FlashBase_S98+loop)) ;
		}while(v1!=v2);
	}
	SetRompage_MODE(gl_currentpage,SYSTEM_MODE_OS);

    REG_IME = saved_ime;
}
//-----------------------------------------------------------
/*void BlueVendor_Chip_Erase()
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
	do
	{
		VBlankIntrWait();		
		VBlankIntrWait();	
		ShowTime(NOR_list,0);
		DrawPic((u16*)(gImage_MENU+78*128*2), 56, 90+13, 128, 13, 0, 0, 1);//show menu pic
		
		itoa(count,msg,2);
		DrawHZText12(msg,0,60,90+13,gl_color_text,1);
		count++;

		VBlankIntrWait();
		v1 = *((vu16 *)(FlashBase_S98)) ;
		v2 = *((vu16 *)(FlashBase_S98)) ;		
	}while(v1!=v2);
	REG_IME = 1 ;
}
*/
//-----------------------------------------------------------
void IWRAM_CODE WriteFlash_with64word(u32 address,u8 *buffer,u32 size)
{
    Blue_PrepareFlashAccess();
    u16 saved_ime = REG_IME;
    REG_IME = 0;

	vu16 page,v1,v2;
	register u32 loopwrite ;
	vu16* buf = (vu16*)buffer ;
	u32 i;

	Chip_Reset();
	page=gl_currentpage;
	while(address>=0x800000)
	{
		address-=0x800000;
		page+=0x1000;
	}
	SetRompage_MODE(page,SYSTEM_MODE_OS);

	//
	v1=0;v2=1;
	for(loopwrite=0;loopwrite<(size/64);loopwrite++)
	{
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
		*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
		*((vu16 *)(FlashBase_S98+address+loopwrite*64)) = 0x25;
		*((vu16 *)(FlashBase_S98+address+loopwrite*64)) = 31;
		for(i=0;i<=31;i++)
		{
			*((vu16 *)(FlashBase_S98+address+loopwrite*64 +2*i )) = buf[loopwrite*32+i];
		}	
		*((vu16 *)(FlashBase_S98+address+loopwrite*64)) = 0x29;
		do
		{
			v1 = *((vu16 *)(FlashBase_S98+address+loopwrite*64)) ;
			v2 = *((vu16 *)(FlashBase_S98+address+loopwrite*64)) ;
		}while(v1!=v2);
		
		//DelayMicroseconds(50);
		
	}

	SetRompage_MODE(gl_currentpage,SYSTEM_MODE_OS);

    REG_IME = saved_ime;
}
//---------------------------------------------------------------
void IWRAM_CODE BlueVendor_WriteFlash_with32word(u32 address,u8 *buffer,u32 size)
{
    Blue_PrepareFlashAccess();
    u16 saved_ime = REG_IME;
    REG_IME = 0;

	vu16 page,v1,v2;
	register u32 loopwrite ;
	vu16* buf = (vu16*)buffer ;
	u32 i;

	page=gl_currentpage;
	while(address>=0x800000)
	{
		address-=0x800000;
		page+=0x1000;
	}
	SetRompage_MODE(page,SYSTEM_MODE_OS);

	//Chip_Reset();
	v1=0;v2=1;
	for(loopwrite=0;loopwrite<(size/32);loopwrite++)
	{
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
		*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
		*((vu16 *)(FlashBase_S98+address+loopwrite*32)) = 0x25;
		*((vu16 *)(FlashBase_S98+address+loopwrite*32)) = 15;
		for(i=0;i<=15;i++)
		{
			*((vu16 *)(FlashBase_S98+address+loopwrite*32 +2*i )) = buf[loopwrite*16+i];
		}	
		*((vu16 *)(FlashBase_S98+address+loopwrite*32)) = 0x29;
		
		do
		{
			v1 = *((vu16 *)(FlashBase_S98+address+loopwrite*32+0xF*2)) ;
			v2 = *((vu16 *)(FlashBase_S98+address+loopwrite*32+0xF*2)) ;
		}while(v1!=v2);
	}
	SetRompage_MODE(gl_currentpage,SYSTEM_MODE_OS);

    REG_IME = saved_ime;
}
//---------------------------------------------------------------
void IWRAM_CODE BlueVendor_WriteFlash(u32 address,u8 *buffer,u32 size)
{
    Blue_PrepareFlashAccess();
    u16 saved_ime = REG_IME;
    REG_IME = 0;

	vu16 page,v1,v2;
	register u32 loopwrite ;
	vu16* buf = (vu16*)buffer ;

	page=gl_currentpage;
	while(address>=0x800000)
	{
		address-=0x800000;
		page+=0x1000;
	}
	SetRompage_MODE(page,SYSTEM_MODE_OS);

	//Chip_Reset();
	v1=0;v2=1;
	for(loopwrite=0;loopwrite<(size/2);loopwrite++)
	{
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
		*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xA0 ;
		*((vu16 *)(FlashBase_S98+address+loopwrite*2)) = buf[loopwrite];
		do
		{
			v1 = *((vu16 *)(FlashBase_S98+address+loopwrite*2)) ;
			v2 = *((vu16 *)(FlashBase_S98+address+loopwrite*2)) ;
		}while(v1!=v2);
	}
	SetRompage_MODE(gl_currentpage,SYSTEM_MODE_OS);

    REG_IME = saved_ime;
}
//-----------------------------------------------------------
u8 IWRAM_CODE Blue_VerifyNorReadback(u32 address, const u8 *expected, u32 size)
{
    if(address >= NOR_end_address || size > NOR_end_address - address ||
       (address | size) & 1 || size > 0x800000 - address % 0x800000) return 0;
    Blue_PrepareFlashAccess();
    u16 incoming = REG_IME;
    REG_IME = 0;
    SetRompage_MODE((address / 0x800000) * 0x1000, SYSTEM_MODE_OS);
    *(vu16 *)FlashBase_S98 = 0xF0;
    volatile u16 *readback = (volatile u16 *)(FlashBase_S98 + address % 0x800000);
    u8 good = 1;
    for(u32 word = 0; word < size / 2; word++) {
        u16 value = expected ? expected[word * 2] | (u16)expected[word * 2 + 1] << 8 : 0xFFFF;
        if(readback[word] != value) { good = 0; break; }
    }
    SetRompage_MODE(gl_currentpage, SYSTEM_MODE_OS);
    REG_IME = incoming;
    return good;
}

u8 Blue_ProgramNorBlock(u32 address, u8 *buffer, u32 size)
{
    if(!buffer || address < First_game_address || address >= NOR_end_address ||
       size > NOR_end_address - address || (address | size) & 63 ||
       size > 0x800000 - address % 0x800000) return 0;
    WriteFlash_with64word(address, buffer, size);
    return Blue_VerifyNorReadback(address, buffer, size);
}

u32 Find_NOR_space(u32 needsize)
{
    return BlueNorFindFree(NOR_FAT_buffer, needsize);
}
//-----------------------------------------------------------
u8 Blue_NorCatalogMatchesMap(void)
{
    u8 occupied[NOR_FAT_SIZE / 8] = {0};
    for(u32 index = 0; index < game_total_NOR; index++) {
        const FM_NOR_FS *live = &pNorFS[index];
        if(!BlueNorValidSpan(live->rompage, live->filesize)) return 0;
        u32 first = live->rompage - First_game_address / 0x20000;
        for(u32 block = first; block < first + live->filesize / 0x20000; block++) {
            if(occupied[block / 8] & (1u << (block % 8))) return 0;
            occupied[block / 8] |= 1u << (block % 8);
        }
    }
    for(u32 block = 0; block < NOR_FAT_SIZE; block++)
        if(!!NOR_FAT_buffer[block] != !!(occupied[block / 8] & (1u << (block % 8))))
            return 0;
    return 1;
}

void Check_NOR_flash(void)
{
    Read_NORgame_info();
    Read_NOR_FAT_info();
    /* Only a completely erased catalog AND map prove a factory-empty device.
       Normalise in RAM; never erase/program metadata while browsing. */
    u32 erased = 1;
    for(u32 i = 0; i < sizeof(pNorFS); i++)
        if(((const u8 *)pNorFS)[i] != 0xFF) erased = 0;
    for(u32 i = 0; i < NOR_FAT_SIZE; i++)
        if(NOR_FAT_buffer[i] != 0xFF) erased = 0;
    if(erased) memset(NOR_FAT_buffer, 0, NOR_FAT_SIZE);
    gl_norOffset = First_game_address;
    game_total_NOR = GetFileListFromNor();
}

//-----------------------------------------------------------
u32 BlueVendor_Loadfile2NOR(TCHAR *filename, u32 NORaddress, u16 have_patch,
                           u8 SAVEMODE, u16 flag_64MBrom)
{
    (void)NORaddress; /* BLUE allocates a free run, not an append-only offset. */
    FIL file;
    FM_NOR_FS entry;
    UINT received = 0;
    u32 result = NOR_WRITE_RESULT_FILE_ERROR;
    u32 allocation;
    u32 filesize;
    u32 destination;
    u32 slot;
    /* Persistent NOR patches must not keep unverified SD state-sector addresses. */
    if(have_patch && gl_rts_on) return result;
    if(!filename || strlen(filename) >= sizeof(entry.filename)) return result;
    if(Read_S98NOR_ID() != NOR_ID) return NOR_WRITE_RESULT_DEVICE_ERROR;
    if(f_open(&file, filename, FA_READ) != FR_OK) return result;
    FSIZE_t full_size = f_size(&file);
    if(full_size < 0xC0 || full_size > PSRAM_CAPACITY_BYTES) goto done;
    filesize = (u32)full_size;
    if(!BlueNorAllocation(filesize, have_patch, iTrimSize, &allocation)) goto done;
    memset(&entry, 0, sizeof(entry));
    if(f_lseek(&file, 0xA0) != FR_OK ||
       f_read(&file, entry.gamename, sizeof(entry.gamename), &received) != FR_OK ||
       received != sizeof(entry.gamename)) goto done;
    Check_NOR_flash();
    if(game_total_NOR >= MAX_NOR) { result = NOR_WRITE_RESULT_FULL; goto done; }
    if(!Blue_NorCatalogMatchesMap()) goto done;
    destination = Find_NOR_space(allocation);
    if(!destination) { result = NOR_WRITE_RESULT_FULL; goto done; }
    slot = game_total_NOR;
    entry.rompage = destination / 0x20000;
    entry.filesize = allocation;
    entry.have_patch = have_patch;
    entry.have_RTS = gl_rts_on;
    entry.is_64MBrom = flag_64MBrom;
    entry.savemode = SAVEMODE;
    strcpy(entry.filename, filename);
    if(f_lseek(&file, 0) != FR_OK) goto done;
    Launcher_DrawNORWriteBackground();
    Chip_Reset();
    for(u32 offset = 0; offset < allocation; offset += MAX_pReadCache_size) {
        u32 bytes = offset < filesize ? filesize - offset : 0;
        if(bytes > MAX_pReadCache_size) bytes = MAX_pReadCache_size;
        if(bytes < MAX_pReadCache_size) memset(pReadCache + bytes, 0xFF, MAX_pReadCache_size - bytes);
        received = 0;
        if(bytes && (f_read(&file, pReadCache, bytes, &received) != FR_OK || received != bytes))
            goto done;
        if(have_patch) {
            if(gl_reset_on || gl_rts_on || gl_sleep_on || gl_cheat_on) {
                PatchInternal((u32 *)pReadCache, MAX_pReadCache_size, offset);
                GBApatch_NOR((u32 *)pReadCache, MAX_pReadCache_size, offset, entry.rompage);
            }
        } else GBApatch_Cleanrom_NOR((u32 *)pReadCache, offset);
        /* Read a complete block before erasing it; preserve vendor command order. */
        if(!Block_Erase(destination + offset)) {
            result = NOR_WRITE_RESULT_ERASE_ERROR; goto done;
        }
        if(!Blue_ProgramNorBlock(destination + offset, pReadCache, MAX_pReadCache_size)) {
            result = NOR_WRITE_RESULT_PROGRAM_ERROR; goto done;
        }
        Blue_DrawNorProgress(offset / MAX_pReadCache_size + 1, allocation / MAX_pReadCache_size);
    }
    if(f_close(&file) != FR_OK) return NOR_WRITE_RESULT_FILE_ERROR;
    pNorFS[slot] = entry;
    if(!Blue_CommitNorCatalog()) {
        memset(&pNorFS[slot], 0, sizeof(entry));
        return NOR_WRITE_RESULT_CATALOG_ERROR;
    }
    gl_norOffset = destination;
    return NOR_WRITE_RESULT_OK;

done:
    if(f_close(&file) != FR_OK) return NOR_WRITE_RESULT_FILE_ERROR;
    return result;
}
//-----------------------------------------------------------
void* IWRAM_CODE my_memcpy(void *destination, const void *source, u32 num)
{
    u8 *dest = destination;
    u8 *src  = (u8*)source;
    while (num) {
        *dest++ = *src++;
        num--;
    }
    return destination;
}

int IWRAM_CODE my_memcmp(void *ptr, const void *ptr2, u32 size)
{
  u8 *p1 = (u8 *)ptr;
  u8 *p2 = (u8 *)ptr2;
  u8 *p3 = (u8 *)ptr + size;

  while(p1 != p3) {
    if (*p1 != *p2) return 1;
    p1++;
    p2++;
  }
    return 0;
}

//-----------------------------------------------------------
u32 IWRAM_CODE GetFileListFromNor(void)
{
    Blue_PrepareFlashAccess();
    u16 incoming_ime = REG_IME;
    u32 count = 0;
    u32 used = 0;
    REG_IME = 0;
    for(u32 slot = 0; slot < MAX_NOR; slot++) {
        FM_NOR_FS *entry = &pNorFS[slot];
        if(!BlueNorValidSpan(entry->rompage, entry->filesize)) continue;
        u32 name_bytes = 0;
        while(name_bytes < sizeof(entry->filename) && entry->filename[name_bytes]) name_bytes++;
        if(!name_bytes || name_bytes == sizeof(entry->filename)) continue;
        u32 start = (u32)entry->rompage << 17;
        u32 overlaps = 0;
        for(u32 prior = 0; prior < count; prior++) {
            u32 other = (u32)pNorFS[prior].rompage << 17;
            if(start < other + pNorFS[prior].filesize && other < start + entry->filesize) overlaps = 1;
        }
        if(overlaps) continue;
        SetRompage_MODE((start / 0x800000) * 0x1000, SYSTEM_MODE_OS);
        uintptr_t header = FlashBase_S98 + start % 0x800000;
        u16 value = *(vu16 *)(header + 0xBE);
        u16 x24 = *(vu16 *)(header + 6);
        u32 valid = ((value & 0xFF) == 0xCE || (value & 0xFF) == 0xCF ||
                     (value & 0xFF) == 0 || x24 == 0x51AE) &&
                    *(vu8 *)(header + 0xB2) == 0x96;
        for(u32 index = 0; index < sizeof(entry->gamename); index++)
            if(*(vu8 *)(header + 0xA0 + index) != (u8)entry->gamename[index]) valid = 0;
        SetRompage_MODE(gl_currentpage, SYSTEM_MODE_OS);
        if(!valid) continue;
        if(count != slot) my_memcpy(&pNorFS[count], entry, sizeof(FM_NOR_FS));
        used += pNorFS[count].filesize;
        count++;
    }
    SetRompage_MODE(gl_currentpage, SYSTEM_MODE_OS);
    for(u32 slot = count; slot < MAX_NOR; slot++)
        for(u32 index = 0; index < sizeof(FM_NOR_FS); index++) ((u8 *)&pNorFS[slot])[index] = 0;
    for(u32 i = 1; i < count; i++) {
        FM_NOR_FS entry = pNorFS[i];
        u32 j = i;
        while(j && pNorFS[j - 1].rompage > entry.rompage) {
            pNorFS[j] = pNorFS[j - 1];
            j--;
        }
        pNorFS[j] = entry;
    }
    gl_norOffset = First_game_address + used;
    REG_IME = incoming_ime;
    return count;
}
