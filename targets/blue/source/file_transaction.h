#ifndef FILE_TRANSACTION_H
#define FILE_TRANSACTION_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "ff.h"

#ifndef EWRAM_BSS
#define FILE_TXN_EWRAM_BSS
#else
#define FILE_TXN_EWRAM_BSS EWRAM_BSS
#endif

/* One synchronous file replacement at a time. All transaction files live in
   /SYSTEM, and the journal records the absolute destination for reboot recovery. */
#define FILE_TXN_DATA_PATH    "/SYSTEM/MXSVTXN.DAT"
#define FILE_TXN_BACKUP_PATH  "/SYSTEM/MXSVTXN.BAK"
#define FILE_TXN_JOURNAL_PATH "/SYSTEM/MXSVTXN.JRN"
#define FILE_TXN_NEW_PATH     "/SYSTEM/MXSVTXN.NEW"
#define FILE_TXN_TARGET_MAX   288
#define FILE_TXN_MAGIC        0x3158544DU

typedef struct
{
	uint32_t magic;
	uint32_t crc32;
	uint32_t target_existed;
	char target[FILE_TXN_TARGET_MAX];
} FileTxnJournal;

static FileTxnJournal file_txn_journal FILE_TXN_EWRAM_BSS;
static char file_txn_active_target[FILE_TXN_TARGET_MAX] FILE_TXN_EWRAM_BSS;
static uint8_t file_txn_active;
static uint8_t file_txn_recovered;
static uint8_t file_txn_target_existed;

static uint32_t FileTxn_Crc32(const uint8_t *data, uint32_t size)
{
	uint32_t crc = 0xFFFFFFFFU;
	uint32_t i;
	uint8_t bit;

	for(i = 0; i < size; i++)
	{
		crc ^= data[i];
		for(bit = 0; bit < 8; bit++)
			crc = (crc >> 1) ^ ((crc & 1U) ? 0xEDB88320U : 0U);
	}
	return crc ^ 0xFFFFFFFFU;
}

static int FileTxn_PathState(const char *path)
{
	FILINFO info;
	FRESULT result = f_stat(path, &info);
	if(result == FR_OK)
		return 1;
	if(result == FR_NO_FILE || result == FR_NO_PATH)
		return 0;
	return -1;
}

static int FileTxn_PathPrefixEqual(const char *path, const char *prefix)
{
	while(*prefix)
	{
		unsigned char a = (unsigned char)*path++;
		unsigned char b = (unsigned char)*prefix++;
		if(a >= 'a' && a <= 'z') a -= 'a' - 'A';
		if(b >= 'a' && b <= 'z') b -= 'a' - 'A';
		if(a != b) return 0;
	}
	return 1;
}

static int FileTxn_IsAllowedTarget(const char *path)
{
	const char *relative = path;
	const char *name;
	if(!relative || !relative[0]) return 0;
	if(relative[1] == ':') relative += 2;
	if(FileTxn_PathPrefixEqual(relative, "/SYSTEM/SETTINGS.TXT") &&
		strlen(relative) == strlen("/SYSTEM/SETTINGS.TXT")) return 1;
	if(!FileTxn_PathPrefixEqual(relative, "/SYSTEM/SAVER/")) return 0;
	name = relative + 14;
	return name[0] && !strchr(name, '/') && !strchr(name, '\\') &&
		strcmp(name, ".") && strcmp(name, "..");
}

static int FileTxn_RemoveIfPresent(const char *path)
{
	int state = FileTxn_PathState(path);
	return state == 0 || (state == 1 && f_unlink(path) == FR_OK);
}

static int FileTxn_ResolveTarget(const char *path, char *resolved, uint32_t capacity)
{
	char current[FILE_TXN_TARGET_MAX];
	size_t path_length;
	size_t current_length;
	int written;

	if(!path || !path[0] || !resolved || capacity == 0)
		return 0;
	if(path[0] == '/' || (path[0] && path[1] == ':'))
	{
		path_length = strlen(path);
		if(path_length >= capacity)
			return 0;
		memmove(resolved, path, path_length + 1);
	}
	else
	{
		if(f_getcwd(current, sizeof(current)) != FR_OK)
			return 0;
		current_length = strlen(current);
		written = snprintf(resolved, capacity, "%s%s%s", current,
			(current_length && current[current_length - 1] == '/') ? "" : "/", path);
		if(written <= 0 || (uint32_t)written >= capacity)
			return 0;
	}

	return FileTxn_IsAllowedTarget(resolved);
}

static int FileTxn_ReadJournal(FileTxnJournal *journal)
{
	FIL file;
	UINT read_size = 0;
	FRESULT result;
	uint32_t expected_crc;

	result = f_open(&file, FILE_TXN_JOURNAL_PATH, FA_READ);
	if(result != FR_OK)
		return 0;
	result = f_read(&file, journal, sizeof(*journal), &read_size);
	if(result != FR_OK || read_size != sizeof(*journal) || f_error(&file))
	{
		(void)f_close(&file);
		return 0;
	}
	result = f_close(&file);
	if(result != FR_OK)
	{
		return 0;
	}
	if(journal->magic != FILE_TXN_MAGIC || journal->target_existed > 1 ||
	   !memchr(journal->target, '\0', sizeof(journal->target)))
	{
		return 0;
	}
	expected_crc = journal->crc32;
	journal->crc32 = 0;
	if(FileTxn_Crc32((const uint8_t*)journal, sizeof(*journal)) != expected_crc)
	{
		return 0;
	}
	journal->crc32 = expected_crc;
	return FileTxn_ResolveTarget(journal->target, journal->target,
		sizeof(journal->target));
}

static int FileTxn_WriteJournal(const char *target)
{
	FIL file;
	UINT written = 0;
	FRESULT result;
	FRESULT close_result;
	int write_error;

	memset(&file_txn_journal, 0, sizeof(file_txn_journal));
	file_txn_journal.magic = FILE_TXN_MAGIC;
	file_txn_journal.target_existed = file_txn_target_existed;
	if(strlen(target) >= sizeof(file_txn_journal.target))
		return 0;
	strcpy(file_txn_journal.target, target);
	file_txn_journal.crc32 = FileTxn_Crc32((const uint8_t*)&file_txn_journal,
		sizeof(file_txn_journal));

	result = f_open(&file, FILE_TXN_NEW_PATH, FA_WRITE | FA_CREATE_NEW);
	if(result != FR_OK)
		return 0;
	result = f_write(&file, &file_txn_journal, sizeof(file_txn_journal), &written);
	write_error = f_error(&file);
	close_result = f_close(&file);
	if(result != FR_OK || written != sizeof(file_txn_journal) || write_error ||
	   close_result != FR_OK)
	{
		(void)FileTxn_RemoveIfPresent(FILE_TXN_NEW_PATH);
		return 0;
	}
	if(f_rename(FILE_TXN_NEW_PATH, FILE_TXN_JOURNAL_PATH) != FR_OK)
	{
		(void)FileTxn_RemoveIfPresent(FILE_TXN_NEW_PATH);
		return 0;
	}
	file_txn_recovered = 0;
	return 1;
}

static int FileTxn_Recover(void)
{
	FRESULT result;
	int journal_state;
	int target_state;
	int backup_state;
	int data_state;

	if(file_txn_active)
	{
		return 0;
	}
	if(file_txn_recovered)
		return 1;
	result = f_mkdir("/SYSTEM");
	if(result != FR_OK && result != FR_EXIST)
	{
		return 0;
	}

	journal_state = FileTxn_PathState(FILE_TXN_JOURNAL_PATH);
	if(journal_state < 0)
	{
		return 0;
	}
	if(journal_state == 0)
	{
		backup_state = FileTxn_PathState(FILE_TXN_BACKUP_PATH);
		if(backup_state != 0)
		{
			return 0;
		}
		if(!FileTxn_RemoveIfPresent(FILE_TXN_DATA_PATH) ||
		   !FileTxn_RemoveIfPresent(FILE_TXN_NEW_PATH))
		{
			return 0;
		}
		file_txn_recovered = 1;
		return 1;
	}

	if(!FileTxn_ReadJournal(&file_txn_journal))
	{
		return 0;
	}
	target_state = FileTxn_PathState(file_txn_journal.target);
	backup_state = FileTxn_PathState(FILE_TXN_BACKUP_PATH);
	data_state = FileTxn_PathState(FILE_TXN_DATA_PATH);
	if(target_state < 0 || backup_state < 0 || data_state < 0)
	{
		return 0;
	}
	if(file_txn_journal.target_existed)
	{
		if(backup_state == 1 && target_state == 0)
		{
			if(f_rename(FILE_TXN_BACKUP_PATH, file_txn_journal.target) != FR_OK)
				return 0;
		}
		else if(backup_state == 1 && target_state == 1 && data_state == 0)
		{
			/* The staged file is installed; retire the old copy. */
			if(!FileTxn_RemoveIfPresent(FILE_TXN_BACKUP_PATH))
				return 0;
		}
		else if(backup_state == 1 && target_state == 1 && data_state == 1)
		{
			/* Interrupted FAT rename can leave duplicate directory entries that
			   share a cluster chain. Preserve all copies for manual recovery. */
			return 0;
		}
		else if(backup_state == 0 && target_state == 1)
		{
			/* The original is still at its destination; staging can be discarded. */
		}
		else
		{
			return 0;
		}
	}
	else if(backup_state != 0)
	{
		return 0;
	}
	else if(target_state == 1 && data_state == 1)
	{
		/* The new destination may share its chain with the staged entry. */
		return 0;
	}
	if(!FileTxn_RemoveIfPresent(FILE_TXN_DATA_PATH) ||
	   !FileTxn_RemoveIfPresent(FILE_TXN_NEW_PATH) ||
	   !FileTxn_RemoveIfPresent(FILE_TXN_JOURNAL_PATH))
	{
		return 0;
	}
	file_txn_recovered = 1;
	return 1;
}

static int FileTxn_Begin(const char *target, FIL *file)
{
	char resolved[FILE_TXN_TARGET_MAX];
	FRESULT result;
	int state;
	int target_state;

	if(!file || file_txn_active ||
	   !FileTxn_ResolveTarget(target, resolved, sizeof(resolved)) ||
	   !FileTxn_Recover())
		return 0;
	state = FileTxn_PathState(FILE_TXN_DATA_PATH);
	if(state != 0 || FileTxn_PathState(FILE_TXN_BACKUP_PATH) != 0 ||
	   FileTxn_PathState(FILE_TXN_NEW_PATH) != 0)
		return 0;
	target_state = FileTxn_PathState(resolved);
	if(target_state < 0)
		return 0;
	result = f_open(file, FILE_TXN_DATA_PATH, FA_WRITE | FA_CREATE_NEW);
	if(result != FR_OK)
		return 0;
	strcpy(file_txn_active_target, resolved);
	file_txn_target_existed = (uint8_t)(target_state == 1);
	file_txn_active = 1;
	return 1;
}

static void FileTxn_Abort(void)
{
	int has_journal = FileTxn_PathState(FILE_TXN_JOURNAL_PATH) == 1;
	file_txn_active = 0;
	if(has_journal)
		(void)FileTxn_Recover();
	else
		(void)FileTxn_RemoveIfPresent(FILE_TXN_DATA_PATH);
}

static int FileTxn_Commit(FIL *file)
{
	FRESULT close_result;
	FRESULT result;
	int target_state;

	if(!file || !file_txn_active)
		return 0;
	if(f_error(file))
	{
		(void)f_close(file);
		FileTxn_Abort();
		return 0;
	}
	close_result = f_close(file);
	if(close_result != FR_OK)
	{
		FileTxn_Abort();
		return 0;
	}
	if(!FileTxn_WriteJournal(file_txn_active_target))
	{
		FileTxn_Abort();
		return 0;
	}

	target_state = FileTxn_PathState(file_txn_active_target);
	if(target_state == 1)
	{
		result = f_rename(file_txn_active_target, FILE_TXN_BACKUP_PATH);
		if(result != FR_OK)
		{
			file_txn_active = 0;
			(void)FileTxn_Recover();
			return 0;
		}
	}
	else if(target_state != 0)
	{
		file_txn_active = 0;
		(void)FileTxn_Recover();
		return 0;
	}

	result = f_rename(FILE_TXN_DATA_PATH, file_txn_active_target);
	file_txn_active = 0;
	if(result != FR_OK)
	{
		/* FatFs rename can leave duplicate entries sharing one cluster chain
		   when directory updates fail. Recovery preserves ambiguous states. */
		(void)FileTxn_Recover();
		return 0;
	}

	/* The new file is complete and installed. Cleanup can be retried on the
	   next file operation if the card rejects deletion of the old copy. */
	(void)FileTxn_Recover();
	return 1;
}

#endif
