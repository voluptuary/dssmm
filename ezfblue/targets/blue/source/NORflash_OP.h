#include <gba_base.h>

#include "ff.h"
//---------------------------------------------------------------
void Chip_Reset();
void BlueVendor_Block_Erase(u32 blockAdd);
void BlueVendor_Chip_Erase();
u8 FormatNor(void);
void BlueVendor_WriteFlash(u32 address,u8 *buffer,u32 size);
void IWRAM_CODE BlueVendor_WriteFlash_with32word(u32 address,u8 *buffer,u32 size);
u32 BlueVendor_Loadfile2NOR(TCHAR *filename, u32 NORaddress,u16 have_patch,u8 SAVEMODE,u16 flag_64MBrom);
u32 GetFileListFromNor(void);
void Check_NOR_flash(void);
u8 Blue_NorCatalogMatchesMap(void);

#include "nor_write_result.h"
u8 Block_Erase(u32 address);
u8 Chip_Erase(void);
u8 WriteFlash(u32 address, u8 *buffer, u32 size);
u8 WriteFlash_with32word(u32 address, u8 *buffer, u32 size);
u32 Loadfile2NOR(TCHAR *filename, const TCHAR *directory, u32 address, u16 patched, u8 save_mode);

u8 Blue_VerifyNorReadback(u32 address, const u8 *expected, u32 size);
u8 Blue_ProgramNorBlock(u32 address, u8 *buffer, u32 size);
