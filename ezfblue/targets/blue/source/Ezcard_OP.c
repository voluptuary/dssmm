#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include <gba_base.h>
#include <gba_dma.h>
#include <string.h>

#include "ez_define.h"
#include "ezkernel.h"
#include "draw.h"
#include "Ezcard_OP.h"


extern u32 FAT_table_buffer[FAT_table_size/4]EWRAM_BSS;

extern FIL gfile;
// --------------------------------------------------------------------
extern void Blue_PrepareFlashAccess(void);
extern void HardReset(void);
extern void SoftReset_now(void);
extern u32 gl_currentpage;

extern u8 NOR_FAT_buffer[NOR_FAT_SIZE]EWRAM_BSS;
// --------------------------------------------------------------------
void IWRAM_CODE SetSDControl(u16  control)
{
	*(u16 *)0x9fe0000 = 0xd200;
	*(u16 *)0x8000000 = 0x1500;
	*(u16 *)0x8020000 = 0xd200;
	*(u16 *)0x8040000 = 0x1500;
	*(u16 *)0x9400000 = control;
	*(u16 *)0x9fc0000 = 0x1500;
}
// --------------------------------------------------------------------
void IWRAM_CODE SD_Enable(void)
{
	SetSDControl(1);
}
// --------------------------------------------------------------------
void IWRAM_CODE SD_Read_state(void)
{
	SetSDControl(3);
}
// --------------------------------------------------------------------
void IWRAM_CODE SD_Disable(void)
{
	SetSDControl(0);
}
// --------------------------------------------------------------------
u16 IWRAM_CODE SD_Response(void)
{
	return *(vu16 *)0x9E00000; 
}
// --------------------------------------------------------------------
u32 IWRAM_CODE Wait_SD_Response()
{
	vu16 res;
	u32 count=0;
	while(1)
	{
		res = SD_Response();
		if(res != 0xEEE1)
		{
			return 0;
		}
			
		count++;
		if(count>0x100000)
		{
			//DEBUG_printf("time out %x",res);	
			//wait_btn();
			return 1;
		}
	}	
}
// --------------------------------------------------------------------
u32 IWRAM_CODE Read_SD_sectors(u32 address,u16 count,u8* SDbuffer)
{
	SD_Enable();
	
	u16 i;
	u16 blocks;
	u32 res;
	u32 times=2;
	for(i=0;i<count;i+=4)
	{
		blocks = (count-i>4)?4:(count-i);
			
	read_again:
		*(vu16 *)0x9fe0000 = 0xd200;
		*(vu16 *)0x8000000 = 0x1500;
		*(vu16 *)0x8020000 = 0xd200;
		*(vu16 *)0x8040000 = 0x1500;
		*(vu16 *)0x9600000 = ((address+i)&0x0000FFFF) ;
		*(vu16 *)0x9620000 = ((address+i)&0xFFFF0000) >>16;
		*(vu16 *)0x9640000 = blocks;
		*(vu16 *)0x9fc0000 = 0x1500;
		SD_Read_state();
		res = Wait_SD_Response();	
		SD_Enable();
		if(res==1)
		{
			times--;
			if(times) 
			{
				delay(5000);
				goto read_again;
			}
			SD_Disable();
			return 1;			
		}
		dmaCopy((void*)0x9E00000, SDbuffer+i*512, blocks*512);
	}
	SD_Disable();
	return 0;
}
// --------------------------------------------------------------------
u32 IWRAM_CODE Write_SD_sectors(u32 address,u16 count, const u8* SDbuffer)
{
	SD_Enable();
	SD_Read_state();
	u16 i;
	u16 blocks;
	u32 res;
	for(i=0;i<count;i+=4)
	{
		blocks = (count-i>4)?4:(count-i);
			
		dmaCopy( SDbuffer+i*512,(void*)0x9E00000, blocks*512);	
		*(vu16 *)0x9fe0000 = 0xd200;
		*(vu16 *)0x8000000 = 0x1500;
		*(vu16 *)0x8020000 = 0xd200;
		*(vu16 *)0x8040000 = 0x1500;
		*(vu16 *)0x9600000 = ((address+i)&0x0000FFFF);
		*(vu16 *)0x9620000 = ((address+i)&0xFFFF0000) >>16;
		*(vu16 *)0x9640000 = 0x8000+blocks;
		*(vu16 *)0x9fc0000 = 0x1500;
				
		res = Wait_SD_Response();
		if(res==1){
			SD_Disable();
			return 1; 
		}
			
	}
	delay(3000);
	SD_Disable();
	return 0;
}
// --------------------------------------------------------------------
/*u16 IWRAM_CODE Read_S71NOR_ID()
{
	u16 ID1;
	*((vu16 *)(FlashBase_S71)) = 0xF0;	
	*((vu16 *)(FlashBase_S71+0x555*2)) = 0xAA;
	*((vu16 *)(FlashBase_S71+0x2AA*2)) = 0x55;
	*((vu16 *)(FlashBase_S71+0x555*2)) = 0x90;
	ID1 = *((vu16 *)(FlashBase_S71+0xE*2));
	*((vu16 *)(FlashBase_S71)) = 0xF0;
	return ID1;
}	*/
// --------------------------------------------------------------------
u16 IWRAM_CODE Read_S98NOR_ID()
{
    Blue_PrepareFlashAccess();
    u16 saved_ime = REG_IME;
    REG_IME = 0;

	u16 ID1;
	*((vu16 *)(FlashBase_S98)) = 0xF0 ;	
	*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA;
	*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55;
	*((vu16 *)(FlashBase_S98+0x555*2)) = 0x90;
	ID1 = *((vu16 *)(FlashBase_S98+0xE*2));
	*((vu16 *)(FlashBase_S98)) = 0xF0 ;	
	REG_IME = saved_ime;
	return ID1;
}	
// --------------------------------------------------------------------
void IWRAM_CODE SetRompage_MODE(u16 page,u16 MODE)
{
	*(vu16 *)0x9fe0000 = 0xd200;
	*(vu16 *)0x8000000 = 0x1500;
	*(vu16 *)0x8020000 = 0xd200;
	*(vu16 *)0x8040000 = 0x1500;
	*(vu16 *)0x9880000 = page+MODE;//C4
	*(vu16 *)0x9fc0000 = 0x1500;
}
// --------------------------------------------------------------------
void  IWRAM_CODE SetbufferControl(u16  control)
{
	*(u16 *)0x9fe0000 = 0xd200;
	*(u16 *)0x8000000 = 0x1500;
	*(u16 *)0x8020000 = 0xd200;
	*(u16 *)0x8040000 = 0x1500;
	*(u16 *)0x9420000 = control;//A1
	*(u16 *)0x9fc0000 = 0x1500;
}
// --------------------------------------------------------------------
void IWRAM_CODE SetPSRampage(u16 page)
{
	*(vu16 *)0x9fe0000 = 0xd200;
	*(vu16 *)0x8000000 = 0x1500;
	*(vu16 *)0x8020000 = 0xd200;
	*(vu16 *)0x8040000 = 0x1500;
	*(vu16 *)0x9860000 = page;//C3
	*(vu16 *)0x9fc0000 = 0x1500;
}
// --------------------------------------------------------------------
void IWRAM_CODE SetRampage(u16 page)
{
	*(vu16 *)0x9fe0000 = 0xd200;
	*(vu16 *)0x8000000 = 0x1500;
	*(vu16 *)0x8020000 = 0xd200;
	*(vu16 *)0x8040000 = 0x1500;
	*(vu16 *)0x9c00000 = page;//E0
	*(vu16 *)0x9fc0000 = 0x1500;
}
// --------------------------------------------------------------------
// --------------------------------------------------------------------
void IWRAM_CODE Send_FATbuffer(u32*buffer,u32 mode)
{	
	SetbufferControl(1);
	dmaCopy(buffer,(void*)0x9E00000, 0x400);
	if(mode==2)
	{
		SetbufferControl(0);
		return;
	}
	SetbufferControl(3);	
	if(mode==1)
	{
		SetbufferControl(0);
		return;
	}
			
	u16 res;
	while(1)
	{
		res = SD_Response();
		if(res != 0x0000)
			break;
	}
	
	while(1)
	{
		res = SD_Response();	
		if(res != 0x0001)
			break;
	}
	SetbufferControl(0);		
}
// --------------------------------------------------------------------
#define	RESET_EWRAM		  (1<<0)	/*!< Clear 256K on-board WRAM			*/
#define	RESET_IWRAM		  (1<<1)	/*!< Clear 32K in-chip WRAM				*/
#define	RESET_PALETTE		(1<<2)	/*!< Clear Palette						*/
#define	RESET_VRAM 	    (1<<3)	/*!< Clear VRAM							*/
#define	RESET_OAM		 	  (1<<4)	/*!< Clear OAM							*/
#define	RESET_SIO		 	  (1<<5)	/*!< Switches to general purpose mode	*/
#define	RESET_SOUND		 	(1<<6)	/*!< Reset Sound registers				*/
#define	RESET_OTHER		 	(1<<7)	/*!< all other registers				*/
// --------------------------------------------------------------------
extern u16 gl_ingame_RTC_open_status;

void IWRAM_CODE SetRompageWithHardReset(u16 page,u32 bootmode)
{
    Blue_PrepareFlashAccess();
    REG_IME = 0;
    Set_RTC_status(gl_ingame_RTC_open_status);
    SetRompage_MODE(page, SYSTEM_MODE_GAME);
    /* No C return or stack access after EWRAM is cleared. */
    Blue_BootHandoff(bootmode);
}
// --------------------------------------------------------------------
void IWRAM_CODE ReadSram(u32 address, u8* data , u32 size )
{
	register unsigned int i ;
	for(i=0;i<size;i++)
	{
		data[i]=*(u8*)(address+i);			
	}
}
// --------------------------------------------------------------------
void IWRAM_CODE WriteSram(u32 address, u8* data , u32 size )
{
	register unsigned int i ;
	for(i=0;i<size;i++)
	{
		*(vu8*)(address+i)=data[i];		
	}
}
// --------------------------------------------------------------------
void IWRAM_CODE Bank_Switching(u8 bank)
{
	*((vu8 *)(SAVE_sram_base+0x5555)) = 0xAA ;
	*((vu8 *)(SAVE_sram_base+0x2AAA)) = 0x55 ;
	*((vu8 *)(SAVE_sram_base+0x5555)) = 0xB0 ;
	*((vu8 *)(SAVE_sram_base+0x0000)) = bank ;	
}
// --------------------------------------------------------------------
u8 IWRAM_CODE Save_info_2_NOR(u32 info_offset, const void * info_buffer,u32 buffersize)
{
    if(!info_buffer || !buffersize || (buffersize & 1) || buffersize > 0x20000 ||
       (info_offset != SAVE_info_offset && info_offset != NOR_info_offset &&
        info_offset != SET_info_offset && info_offset != NOR_FAT_offset)) return 0;
    Blue_PrepareFlashAccess();
    u16 saved_ime = REG_IME;
    REG_IME = 0;

	u32 offset;
	const vu16* buf = (const vu16*)info_buffer ;
	register u32 loopwrite ;
	vu16 v1,v2;
	SetRompage_MODE(gl_currentpage,SYSTEM_MODE_OS);
	
	*((vu16 *)(FlashBase_S98)) = 0xF0 ;	
	
	offset= info_offset;//0x780000 0x7A0000/0x7C0000 ; S98 128KB /sec
	
	*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
	*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
	*((vu16 *)(FlashBase_S98+0x555*2)) = 0x80 ;
	*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
	*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;	
	*((vu16 *)(FlashBase_S98+offset)) = 0x30 ;//erase
	do
	{
		v1 = *((vu16 *)(FlashBase_S98+offset)) ;
		v2 = *((vu16 *)(FlashBase_S98+offset)) ;
	}while(v1!=v2);		
	//erase finish

	for(loopwrite=0;loopwrite<buffersize/2;loopwrite++)
	{
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xAA ;
		*((vu16 *)(FlashBase_S98+0x2AA*2)) = 0x55 ;
		*((vu16 *)(FlashBase_S98+0x555*2)) = 0xA0 ;
		*((vu16 *)(FlashBase_S98+offset+loopwrite*2)) = buf[loopwrite];
		do
		{
			v1 = *((vu16 *)(FlashBase_S98+offset+loopwrite*2)) ;
			v2 = *((vu16 *)(FlashBase_S98+offset+loopwrite*2)) ;
		}while(v1!=v2);
	}			

	*((vu16 *)(FlashBase_S98)) = 0xF0;	

    u8 good = 1;
    for(u32 index = 0; index < buffersize / 2; index++)
        if(*(vu16 *)(FlashBase_S98 + info_offset + index * 2) != buf[index]) { good = 0; break; }
    REG_IME = saved_ime;
    return good;
}

// --------------------------------------------------------------------
u8 IWRAM_CODE Save_sav_info(u16 * SAV_info_buffer,u32 buffersize)
{
	return Save_info_2_NOR(SAVE_info_offset, SAV_info_buffer,buffersize);
}
// --------------------------------------------------------------------
u8 IWRAM_CODE Save_NORgame_info(u16 * NOR_info_buffer,u32 buffersize)
{
	return Save_info_2_NOR(NOR_info_offset, NOR_info_buffer,buffersize);
}
// --------------------------------------------------------------------
void IWRAM_CODE Save_SET_info(u16 * SET_info_buffer,u32 buffersize)
{
	Save_info_2_NOR(SET_info_offset, SET_info_buffer,buffersize);
}
// --------------------------------------------------------------------
void IWRAM_CODE Read_NORgame_info(void)
{
	u16 *dst = (u16*)pNorFS;
	const u32 words = sizeof(pNorFS) / (sizeof(u16));
	
   for (u32 i = 0; i < words; i++)
       dst[i] = *(vu16*)(FlashBase_S98 + NOR_info_offset + i * 2);
}
// --------------------------------------------------------------------
u16 IWRAM_CODE Read_SET_info(u32 offset)
{
	return *((vu16 *)(FlashBase_S98+SET_info_offset+offset*2));
}
// --------------------------------------------------------------------
u16 IWRAM_CODE Read_sav_info(u32 offset)
{
	return *((vu16 *)(FlashBase_S98+SAVE_info_offset+offset*2));
}
// --------------------------------------------------------------------
void IWRAM_CODE Read_NOR_FAT_info(void)
{
	register u32 loopwrite ;
	for(loopwrite=0;loopwrite<NOR_FAT_SIZE/2;loopwrite++)
	{
		((u16*)NOR_FAT_buffer)[loopwrite] = *((vu16 *)(FlashBase_S98+NOR_FAT_offset+loopwrite*2));
	}
}
// --------------------------------------------------------------------
void IWRAM_CODE Set_NOR_FAT_info(u32 offset,u8 v)
{
	NOR_FAT_buffer[offset] = v;
}
// --------------------------------------------------------------------
u8 IWRAM_CODE Get_NOR_FAT_info(u32 offset)
{
	return NOR_FAT_buffer[offset];
}
// --------------------------------------------------------------------
u8 IWRAM_CODE Save_NOR_FAT_info(void)
{
	return Save_info_2_NOR(NOR_FAT_offset, NOR_FAT_buffer,NOR_FAT_SIZE);
}

// --------------------------------------------------------------------
void IWRAM_CODE SetSPIControl(u16  control)
{
	*(u16 *)0x9fe0000 = 0xd200;
	*(u16 *)0x8000000 = 0x1500;
	*(u16 *)0x8020000 = 0xd200;
	*(u16 *)0x8040000 = 0x1500;
	*(u16 *)0x9660000 = control;
	*(u16 *)0x9fc0000 = 0x1500;
}
// --------------------------------------------------------------------
void IWRAM_CODE SPI_Enable(void)
{
	SetSPIControl(1);
}
// --------------------------------------------------------------------
void IWRAM_CODE SPI_Disable(void)
{
	SetSPIControl(0);
}
// --------------------------------------------------------------------
u16 IWRAM_CODE Read_FPGA_ver(void)
{
	u16 Read_SPI;
	//SetRompage_MODE(0x0000,SYSTEM_MODE_OS);//select flash0
	SPI_Enable();	
	Read_SPI =  *(vu16 *)0x9E00000; 
	SPI_Disable();
	//SetRompage_MODE(0x0040,SYSTEM_MODE_GAME);//select flash0
	return Read_SPI;
}
// --------------------------------------------------------------------
void IWRAM_CODE SetSPIWrite(u16  control)
{
	*(u16 *)0x9fe0000 = 0xd200;
	*(u16 *)0x8000000 = 0x1500;
	*(u16 *)0x8020000 = 0xd200;
	*(u16 *)0x8040000 = 0x1500;
	*(u16 *)0x9680000 = control;
	*(u16 *)0x9fc0000 = 0x1500;
}
// --------------------------------------------------------------------
void IWRAM_CODE SPI_Write_Enable(void)
{
	SetSPIWrite(1);
}
// --------------------------------------------------------------------
void IWRAM_CODE SPI_Write_Disable(void)
{
	SetSPIWrite(0);
}
// --------------------------------------------------------------------
void IWRAM_CODE Set_RTC_status(u16  status)
{
	*(u16 *)0x9fe0000 = 0xd200;
	*(u16 *)0x8000000 = 0x1500;
	*(u16 *)0x8020000 = 0xd200;
	*(u16 *)0x8040000 = 0x1500;
	*(u16 *)0x96A0000 = status;
	*(u16 *)0x9fc0000 = 0x1500;
}
// --------------------------------------------------------------------
/*void IWRAM_CODE Set_AUTO_save(u16  mode)
{
	*(u16 *)0x9fe0000 = 0xd200;
	*(u16 *)0x8000000 = 0x1500;
	*(u16 *)0x8020000 = 0xd200;
	*(u16 *)0x8040000 = 0x1500;
	*(u16 *)0x96C0000 = mode;
	*(u16 *)0x9fc0000 = 0x1500;
}*/
// --------------------------------------------------------------------
void IWRAM_CODE Set_LED_control(u16  status)
{
	*(u16 *)0x9fe0000 = 0xd200;
	*(u16 *)0x8000000 = 0x1500;
	*(u16 *)0x8020000 = 0xd200;
	*(u16 *)0x8040000 = 0x1500;
	*(u16 *)0x96E0000 = status;
	*(u16 *)0x9fc0000 = 0x1500;
}
// --------------------------------------------------------------------
void IWRAM_CODE Set_64MROM_flag(u16  flag)
{
	*(u16 *)0x9fe0000 = 0xd200;
	*(u16 *)0x8000000 = 0x1500;
	*(u16 *)0x8020000 = 0xd200;
	*(u16 *)0x8040000 = 0x1500;
	*(u16 *)0x9700000 = flag;
	*(u16 *)0x9fc0000 = 0x1500;
}
// --------------------------------------------------------------------
/* FPGA updates are intentionally handled by the official BLUE kernel. */
