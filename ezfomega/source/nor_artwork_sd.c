/* Original OMEGA artwork hints live on SD, never in cartridge metadata. */
#include <gba_base.h>
#include <stdio.h>
#include <string.h>
#include "ezkernel.h"
#include "nor_artwork_sd.h"

static char omega_art_path[MAX_path_len * 2 + 2] EWRAM_BSS;

/* This is artwork-only SD metadata, never a new cartridge flash allocation. */
typedef struct {
    u32 magic;
    u32 identity;
    u16 length;
    u16 reserved;
    char path[MAX_path_len * 2 + 2];
    u32 checksum;
} OmegaNorArtworkRecord;

static u32 Omega_Hash(const void *bytes, u32 size)
{
    const u8 *data = bytes;
    u32 value = 2166136261UL;
    while(size--) value = (value ^ *data++) * 16777619UL;
    return value;
}

static u32 Omega_ArtworkIdentity(u32 slot)
{
    /* Includes rompage, allocation length, name and game identity. */
    return Omega_Hash(&pNorFS[slot], sizeof(FM_NOR_FS));
}

static void Omega_ArtworkFilename(char *name, u32 slot)
{
    snprintf(name, 48, "/SYSTEM/NORART/%04x.oma", pNorFS[slot].rompage);
}

void Omega_SaveNorArtworkPath(u32 slot, const char *path)
{
    char name[48];
    OmegaNorArtworkRecord record;
    FIL file;
    UINT written = 0;
    if(slot >= MAX_NOR || !path || strlen(path) >= sizeof(record.path)) return;
    if(path[0] && path[0] != '/') return;
    if(f_mkdir("/SYSTEM") != FR_OK && f_stat("/SYSTEM", 0) != FR_OK) return;
    if(f_mkdir("/SYSTEM/NORART") != FR_OK && f_stat("/SYSTEM/NORART", 0) != FR_OK) return;
    Omega_ArtworkFilename(name, slot);
    if(!path[0]) { f_unlink(name); return; }
    memset(&record, 0, sizeof(record));
    record.magic = 0x3152414F; /* OAR1 */
    record.identity = Omega_ArtworkIdentity(slot);
    record.length = strlen(path);
    memcpy(record.path, path, record.length);
    record.checksum = Omega_Hash(&record, sizeof(record) - sizeof(record.checksum));
    if(f_open(&file, name, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return;
    FRESULT result = f_write(&file, &record, sizeof(record), &written);
    FRESULT closed = f_close(&file);
    if(result != FR_OK || closed != FR_OK || written != sizeof(record)) f_unlink(name);
}

const char *Omega_ReadNorArtworkPath(u32 slot)
{
    char name[48];
    OmegaNorArtworkRecord record;
    FIL file;
    UINT received = 0;
    omega_art_path[0] = 0;
    if(slot >= MAX_NOR || !pNorFS[slot].filesize) return 0;
    Omega_ArtworkFilename(name, slot);
    if(f_open(&file, name, FA_READ) != FR_OK) return 0;
    if(f_size(&file) != sizeof(record)) { f_close(&file); return 0; }
    FRESULT result = f_read(&file, &record, sizeof(record), &received);
    FRESULT closed = f_close(&file);
    if(result != FR_OK || closed != FR_OK || received != sizeof(record) ||
       record.magic != 0x3152414F || record.identity != Omega_ArtworkIdentity(slot) ||
       !record.length || record.length >= sizeof(record.path) || record.path[0] != '/' ||
       record.path[record.length] ||
       record.checksum != Omega_Hash(&record, sizeof(record) - sizeof(record.checksum))) return 0;
    memcpy(omega_art_path, record.path, record.length + 1);
    return omega_art_path;
}

