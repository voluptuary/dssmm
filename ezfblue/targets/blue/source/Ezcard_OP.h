#include <gba_base.h>

#include "ff.h"

void IWRAM_CODE SD_Disable(void);
void  SetSDControl(u16  control);
u16 IWRAM_CODE SD_Response(void);
u32 Read_SD_sectors(u32 address,u16 count,u8* SDbuffer);
u32 Write_SD_sectors(u32 address,u16 count,const u8* SDbuffer);
u16 IWRAM_CODE Read_S71NOR_ID();
u16 Read_S98NOR_ID();
void IWRAM_CODE SetRompage(u16 page);
void  IWRAM_CODE SetbufferControl(u16  control);
void SetPSRampage(u16 page);
void SetRampage(u16 page);
void IWRAM_CODE Progress(u16 x, u16 y, u16 w, u16 h, u16 c, u8 isDrawDirect);
void IWRAM_CODE Send_FATbuffer(u32*buffer,u32 mode);
void IWRAM_CODE SetRompageWithHardReset(u16 page,u32 bootmode) __attribute__((noreturn));
void Blue_BootNative(u16 page, u32 hard_reset) __attribute__((noreturn));
void Blue_BootHandoff(u32 mode) __attribute__((noreturn));
void ReadSram(u32 address, u8* data , u32 size );
void WriteSram(u32 address, u8* data , u32 size );
u8 IWRAM_CODE Save_sav_info(u16 * SAV_info_buffer,u32 buffersize);
u8 IWRAM_CODE Save_NORgame_info(u16 * NOR_info_buffer,u32 buffersize);
void IWRAM_CODE Save_SET_info(u16 * SET_info_buffer,u32 buffersize);
void IWRAM_CODE Read_NORgame_info();
u16 IWRAM_CODE Read_SET_info(u32 offset);
u32 Loadfile2PSRAM(TCHAR *filename);
u16 IWRAM_CODE Read_FPGA_ver(void);
void IWRAM_CODE Set_RTC_status(u16  status);
void IWRAM_CODE Set_LED_control(u16  status);
void IWRAM_CODE Set_64MROM_flag(u16  flag);
void IWRAM_CODE Set_DDR_flag(u16  flag);


u16 IWRAM_CODE Read_sav_info(u32 offset);
void IWRAM_CODE Bank_Switching(u8 bank);
void IWRAM_CODE SetRompage_MODE(u16 page, u16 mode);
void IWRAM_CODE Read_NOR_FAT_info(void);
u8 IWRAM_CODE Save_NOR_FAT_info(void);
void IWRAM_CODE Set_NOR_FAT_info(u32 offset, u8 value);
u8 IWRAM_CODE Get_NOR_FAT_info(u32 offset);

void Read_NOR_info(void);
u8 Save_NOR_info_with_art_path(u16 *buffer, u32 bytes, u32 slot, const char *path, u8 clear_paths);
u8 Save_NOR_info(u16 *buffer, u32 bytes);
const char *Read_NOR_art_path(u32 slot);
void Blue_SaveNorArtworkPath(u32 slot, const char *path);
u8 Blue_CommitNorCatalog(void);

u8 Blue_ValidateKernelCandidate(const char *path);

u8 Blue_ReadLaunchMetadata(const char *name, u32 is_plugin, u32 *bytes, u8 code[4], u16 *flag64);
