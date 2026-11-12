/* DS Style interfaces over the BLUE vendor hardware/metadata format. */
#include <gba_base.h>
#include <gba_input.h>
#include <gba_systemcalls.h>
#include <stdio.h>
#include <string.h>
#include "ez_define.h"
#include "ezkernel.h"
#include "Ezcard_OP.h"
#include "NORflash_OP.h"
#include "draw.h"
#include "blue_nor_policy.h"
#include "kernel_boot_policy.h"

extern u32 game_total_NOR;
extern u32 gl_norOffset;
extern u32 gl_currentpage;
extern void Blue_DrawFormatPanel(void);
extern void Blue_DrawNorProgress(u32 completed, u32 total);
extern void UIAudio_StopForSharedBufferUse(void);

static u8 blue_planned_map[NOR_FAT_SIZE] EWRAM_BSS;
static char blue_art_path[MAX_path_len] EWRAM_BSS;

/* This is artwork-only SD metadata, never a new cartridge flash allocation. */
typedef struct {
    u32 magic;
    u32 identity;
    u16 length;
    u16 reserved;
    char path[MAX_path_len];
    u32 checksum;
} BlueNorArtworkRecord;

static u32 Blue_Hash(const void *bytes, u32 size)
{
    const u8 *data = bytes;
    u32 value = 2166136261UL;
    while(size--) value = (value ^ *data++) * 16777619UL;
    return value;
}

static u32 Blue_ArtworkIdentity(u32 slot)
{
    /* Includes rompage, allocation length, name and game identity. */
    return Blue_Hash(&pNorFS[slot], sizeof(FM_NOR_FS));
}

static void Blue_ArtworkFilename(char *name, u32 slot)
{
    snprintf(name, 48, "/SYSTEM/NORART/%04x.art", pNorFS[slot].rompage);
}

void Blue_SaveNorArtworkPath(u32 slot, const char *path)
{
    char name[48];
    BlueNorArtworkRecord record;
    FIL file;
    UINT written = 0;
    if(slot >= MAX_NOR || !path || strlen(path) >= sizeof(record.path)) return;
    if(path[0] && path[0] != '/') return;
    if(f_mkdir("/SYSTEM") != FR_OK && f_stat("/SYSTEM", 0) != FR_OK) return;
    if(f_mkdir("/SYSTEM/NORART") != FR_OK && f_stat("/SYSTEM/NORART", 0) != FR_OK) return;
    Blue_ArtworkFilename(name, slot);
    if(!path[0]) { f_unlink(name); return; }
    memset(&record, 0, sizeof(record));
    record.magic = 0x31504E42; /* BNP1 */
    record.identity = Blue_ArtworkIdentity(slot);
    record.length = strlen(path);
    memcpy(record.path, path, record.length);
    record.checksum = Blue_Hash(&record, sizeof(record) - sizeof(record.checksum));
    if(f_open(&file, name, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return;
    FRESULT result = f_write(&file, &record, sizeof(record), &written);
    FRESULT closed = f_close(&file);
    if(result != FR_OK || closed != FR_OK || written != sizeof(record)) f_unlink(name);
}

const char *Read_NOR_art_path(u32 slot)
{
    char name[48];
    BlueNorArtworkRecord record;
    FIL file;
    UINT received = 0;
    blue_art_path[0] = 0;
    if(slot >= MAX_NOR || !pNorFS[slot].filesize) return 0;
    Blue_ArtworkFilename(name, slot);
    if(f_open(&file, name, FA_READ) != FR_OK) return 0;
    if(f_size(&file) != sizeof(record)) { f_close(&file); return 0; }
    FRESULT result = f_read(&file, &record, sizeof(record), &received);
    FRESULT closed = f_close(&file);
    if(result != FR_OK || closed != FR_OK || received != sizeof(record) ||
       record.magic != 0x31504E42 || record.identity != Blue_ArtworkIdentity(slot) ||
       !record.length || record.length >= sizeof(record.path) || record.path[0] != '/' ||
       record.path[record.length] ||
       record.checksum != Blue_Hash(&record, sizeof(record) - sizeof(record.checksum))) return 0;
    memcpy(blue_art_path, record.path, record.length + 1);
    return blue_art_path;
}

void Read_NOR_info(void) { Read_NORgame_info(); }

u8 Blue_CommitNorCatalog(void)
{
    memset(blue_planned_map, 0, sizeof(blue_planned_map));
    for(u32 slot = 0; slot < MAX_NOR; slot++) {
        const FM_NOR_FS *entry = &pNorFS[slot];
        if(!entry->filesize) continue;
        if(!BlueNorMark(blue_planned_map, entry->rompage, entry->filesize)) return 0;
    }
    /* Reserve new pages before publishing them; release deleted pages afterward.
       Interrupted installation can leak a reservation, never advertise live pages
       as free to the stock allocation-map reader. This is not catalog atomicity. */
    u8 reserve_changed = 0;
    for(u32 page = 0; page < NOR_FAT_SIZE; page++) {
        if(blue_planned_map[page] && !NOR_FAT_buffer[page]) {
            NOR_FAT_buffer[page] = 0xFF;
            reserve_changed = 1;
        }
    }
    if(reserve_changed && !Save_NOR_FAT_info()) return 0;
    if(!Save_NORgame_info((u16 *)pNorFS, sizeof(FM_NOR_FS) * MAX_NOR)) return 0;
    if(memcmp(NOR_FAT_buffer, blue_planned_map, sizeof(blue_planned_map))) {
        memcpy(NOR_FAT_buffer, blue_planned_map, sizeof(blue_planned_map));
        if(!Save_NOR_FAT_info()) return 0;
    }
    return 1; /* Readback verified; catalog interruption is still not atomic. */
}

u8 Save_NOR_info(u16 *buffer, u32 bytes)
{
    if(buffer != (u16 *)pNorFS || bytes != sizeof(FM_NOR_FS) * MAX_NOR) return 0;
    return Blue_CommitNorCatalog();
}

u8 Save_NOR_info_with_art_path(u16 *buffer, u32 bytes, u32 slot,
                             const char *path, u8 clear_paths)
{
    (void)clear_paths; /* Stale SD records are rejected by their catalog identity. */
    if(!Save_NOR_info(buffer, bytes)) return 0;
    if(path && slot < MAX_NOR) Blue_SaveNorArtworkPath(slot, path);
    return 1;
}

u8 Block_Erase(u32 address)
{
    if(address < First_game_address || address >= NOR_end_address || address % 0x20000) return 0;
    BlueVendor_Block_Erase(address);
    return Blue_VerifyNorReadback(address, 0, 0x20000);
}

u8 Chip_Erase(void)
{
    /* Never call the vendor whole-chip erase on a kernel/metadata-bearing die. */
    for(u32 address = First_game_address; address < NOR_end_address; address += 0x20000)
        if(!Block_Erase(address)) return 0;
    return 1;
}

u8 WriteFlash(u32 address, u8 *buffer, u32 size)
{
    if(!buffer || address < First_game_address || address >= NOR_end_address ||
       size > NOR_end_address - address || (address | size) & 1) return 0;
    if(size > 0x800000 - address % 0x800000) return 0;
    BlueVendor_WriteFlash(address, buffer, size);
    return Blue_VerifyNorReadback(address, buffer, size);
}

u8 WriteFlash_with32word(u32 address, u8 *buffer, u32 size)
{
    if(!buffer || address < First_game_address || address >= NOR_end_address ||
       size > NOR_end_address - address || (address | size) & 31) return 0;
    if(size > 0x800000 - address % 0x800000) return 0;
    BlueVendor_WriteFlash_with32word(address, buffer, size);
    return Blue_VerifyNorReadback(address, buffer, size);
}

u32 Loadfile2NOR(TCHAR *filename, const TCHAR *directory, u32 address, u16 patched, u8 save_mode)
{
    FIL file;
    UINT received = 0;
    char flag[2];
    u16 flag64 = 0;
    if(f_open(&file, filename, FA_READ) != FR_OK) return NOR_WRITE_RESULT_FILE_ERROR;
    FSIZE_t size = f_size(&file);
    FRESULT result = f_lseek(&file, 0xB0);
    if(result == FR_OK) result = f_read(&file, flag, sizeof(flag), &received);
    FRESULT closed = f_close(&file);
    if(result != FR_OK || closed != FR_OK || received != sizeof(flag) || size < 0xC0 ||
       size > PSRAM_CAPACITY_BYTES) return NOR_WRITE_RESULT_FILE_ERROR;
    flag64 = size > 0x02000000 && !memcmp(flag, "5G", 2);
    u32 load_result = BlueVendor_Loadfile2NOR(filename, address, patched, save_mode, flag64);
    if(load_result == NOR_WRITE_RESULT_OK) {
        for(u32 slot = 0; slot < MAX_NOR; slot++)
            if(pNorFS[slot].rompage == gl_norOffset / 0x20000 &&
               !strcmp(pNorFS[slot].filename, filename)) {
                Blue_SaveNorArtworkPath(slot, directory);
                break;
            }
    }
    return load_result;
}

u8 FormatNor(void)
{
    Blue_DrawFormatPanel();
    for(;;) {
        VBlankIntrWait(); scanKeys();
        if(keysDown() & KEY_B) return 0;
        if(keysDown() & KEY_A) break;
    }
    UIAudio_StopForSharedBufferUse();
    for(u32 address = First_game_address; address < NOR_end_address; address += 0x20000) {
        if(!Block_Erase(address)) return 2;
        Blue_DrawNorProgress((address - First_game_address) / 0x20000 + 1, NOR_FAT_SIZE);
    }
    memset(pNorFS, 0, sizeof(FM_NOR_FS) * MAX_NOR);
    if(!Blue_CommitNorCatalog()) return 2;
    game_total_NOR = 0;
    gl_norOffset = First_game_address;
    return 1;
}

u8 Blue_ValidateKernelCandidate(const char *path)
{
    FIL file;
    UINT received = 0;
    u8 fixed;
    u32 crc = 0xFFFFFFFFUL;
    if(!path || f_open(&file, path, FA_READ) != FR_OK) return 0;
    FSIZE_t size = f_size(&file);
    if(size < 0x2069DC || size > SAVE_info_offset ||
       f_lseek(&file, 0xB2) != FR_OK ||
       f_read(&file, &fixed, 1, &received) != FR_OK || received != 1 || fixed != 0x96 ||
       f_lseek(&file, 0x195000) != FR_OK) {
        f_close(&file); return 0;
    }
    for(u32 offset = 0; offset < LX16_newomega_top_bin_size;) {
        u32 bytes = LX16_newomega_top_bin_size - offset;
        if(bytes > MAX_pReadCache_size) bytes = MAX_pReadCache_size;
        received = 0;
        if(f_read(&file, pReadCache, bytes, &received) != FR_OK || received != bytes) {
            f_close(&file); return 0;
        }
        for(u32 index = 0; index < bytes; index++) {
            crc ^= pReadCache[index];
            for(u32 bit = 0; bit < 8; bit++)
                crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320UL : 0);
        }
        offset += bytes;
    }
    FRESULT closed = f_close(&file);
    crc = ~crc;
    return closed == FR_OK && crc == (u32)LX16_FW_crc32;
}

void Blue_BootNative(u16 page, u32 hard_reset)
{
    /* Mode 6 preserves BLUE's native hard-reset EWRAM-clear behavior. */
    SetRompageWithHardReset(page, hard_reset ? 6 : 0);
}

u8 Blue_ReadLaunchMetadata(const char *name, u32 is_plugin, u32 *bytes,
                          u8 gamecode[4], u16 *flag64)
{
    FIL file;
    u8 header[0xC0];
    UINT received = 0;
    if(!name || !bytes || !gamecode || !flag64) return 0;
    *bytes = 0; *flag64 = 0;
    memset(gamecode, 'F', 4);
    if(f_open(&file, name, FA_READ) != FR_OK) return 0;
    FSIZE_t size = f_size(&file);
    u8 good = size && size <= (is_plugin ? KERNEL_PLUGIN_ROM_LIMIT : PSRAM_CAPACITY_BYTES);
    if(good && !is_plugin) {
        good = size >= sizeof(header) &&
               f_read(&file, header, sizeof(header), &received) == FR_OK && received == sizeof(header);
        if(good) {
            memcpy(gamecode, header + 0xAC, 4);
            *flag64 = size > 0x02000000 && !memcmp(header + 0xB0, "5G", 2);
        }
    }
    FRESULT closed = f_close(&file);
    if(!good || closed != FR_OK) return 0;
    *bytes = (u32)size;
    return 1;
}
