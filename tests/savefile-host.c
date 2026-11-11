#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ff.h"
#include "diskio.h"
#include "savefile_policy.h"

typedef uint32_t u32;
typedef uint8_t u8;
#define IWRAM_CODE
static u8 pReadCache[0x20000];

#include "file_transaction.h"

/* The host test includes the exact SavefileWrite() body extracted from firmware. */
#include SAVEFILEWRITE_IMPL

#define TEST_SECTORS 4096U
#define SECTOR_SIZE 512U
static BYTE disk[TEST_SECTORS * SECTOR_SIZE];
static BYTE formatted_disk[TEST_SECTORS * SECTOR_SIZE];
static FATFS fs;
static long fail_at = -1;
static long write_calls;
static int injected_failure;
static int inject_next_sram_write;
static int failures;
static const char long_save_path[] = "/SYSTEM/SAVER/Omega Definitive Save Creation Regression.sav";

#define SRAMSaver 0U
void SetRampage(uint16_t page) { (void)page; }
void ReadSram(uint32_t address, uint8_t *data, uint32_t size)
{
	(void)address;
	memset(data, 0x5A, size);
	if(inject_next_sram_write)
	{
		fail_at = write_calls;
		inject_next_sram_write = 0;
	}
}

/* The host test also includes the exact Save_savefile() body. */
#include SAVESAVEFILE_IMPL

#define CHECK(condition, message) do { \
	if(condition) printf("  ok    %s\n", message); \
	else { printf("  FAIL  %s\n", message); failures++; } \
} while(0)

DSTATUS disk_initialize(BYTE pdrv) { return pdrv == 0 ? 0 : STA_NOINIT; }
DSTATUS disk_status(BYTE pdrv) { return pdrv == 0 ? 0 : STA_NOINIT; }

DRESULT disk_read(BYTE pdrv, BYTE *buffer, LBA_t sector, UINT count)
{
	if(pdrv != 0 || sector > TEST_SECTORS || count > TEST_SECTORS - sector)
		return RES_PARERR;
	memcpy(buffer, disk + (size_t)sector * SECTOR_SIZE, (size_t)count * SECTOR_SIZE);
	return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buffer, LBA_t sector, UINT count)
{
	long call = write_calls++;
	if(pdrv != 0 || sector > TEST_SECTORS || count > TEST_SECTORS - sector)
		return RES_PARERR;
	if(call == fail_at)
	{
		injected_failure = 1;
		return RES_ERROR;
	}
	memcpy(disk + (size_t)sector * SECTOR_SIZE, buffer, (size_t)count * SECTOR_SIZE);
	return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE command, void *buffer)
{
	if(pdrv != 0)
		return RES_PARERR;
	switch(command)
	{
		case CTRL_SYNC: return RES_OK;
		case GET_SECTOR_COUNT: *(LBA_t *)buffer = TEST_SECTORS; return RES_OK;
		case GET_SECTOR_SIZE: *(WORD *)buffer = SECTOR_SIZE; return RES_OK;
		case GET_BLOCK_SIZE: *(DWORD *)buffer = 1; return RES_OK;
		default: return RES_PARERR;
	}
}

DWORD get_fattime(void)
{
	return ((DWORD)(2026 - 1980) << 25) | ((DWORD)10 << 21) | ((DWORD)16 << 16);
}

static void check(FRESULT result, const char *message)
{
	if(result != FR_OK)
	{
		fprintf(stderr, "FAIL: %s (FatFs result %u)\n", message, (unsigned)result);
		exit(2);
	}
}

static void mount_fresh_formatted_volume(void)
{
	check(f_mount(NULL, "0:", 0), "unmount before restoring test volume");
	memcpy(disk, formatted_disk, sizeof(disk));
	check(f_mount(&fs, "0:", 1), "mount test volume");
}

static void format_volume(void)
{
	MKFS_PARM options;
	BYTE work[4096];
	memset(disk, 0, sizeof(disk));
	memset(&options, 0, sizeof(options));
	options.fmt = FM_FAT | FM_SFD;
	check(f_mkfs("0:", &options, work, sizeof(work)), "format in-memory FAT volume");
	check(f_mount(&fs, "0:", 1), "mount formatted volume");
	check(f_mkdir("/SYSTEM"), "create system directory");
	check(f_mkdir("/SYSTEM/SAVER"), "create save directory");
	memcpy(formatted_disk, disk, sizeof(disk));
}

static void test_new_save_creation(void)
{
	FIL file;
	FILINFO info;
	FRESULT result;
	BYTE contents[0x8000];
	UINT bytes_read = 0;

	result = f_open(&file, "0:/MISSING.SAV", FA_OPEN_EXISTING);
	CHECK(result == FR_NO_FILE && SaveFileOpenMeansMissing(result),
		"missing file is the only open result that permits save creation");
	result = f_open(&file, "0:/MISSING_DIR/MISSING.SAV", FA_OPEN_EXISTING);
	CHECK(result == FR_NO_PATH && !SaveFileOpenMeansMissing(result),
		"missing directory is not mistaken for a missing save");
	CHECK(!SaveFileOpenMeansMissing(FR_DISK_ERR),
		"disk I/O error is not mistaken for a missing save");

	CHECK(SavefileWrite("/SYSTEM/SAVER/OK.SAV", sizeof(contents)) == 1,
		"successful first-time save returns success");
	result = f_stat("/SYSTEM/SAVER/OK.SAV", &info);
	CHECK(result == FR_OK && info.fsize == sizeof(contents),
		"successful save has the requested full size");
	result = f_open(&file, "/SYSTEM/SAVER/OK.SAV", FA_READ);
	if(result == FR_OK)
	{
		result = f_read(&file, contents, sizeof(contents), &bytes_read);
		CHECK(result == FR_OK && bytes_read == sizeof(contents),
			"read back the complete newly created save");
		if(result == FR_OK && bytes_read == sizeof(contents))
		{
			size_t i;
			int all_erased = 1;
			for(i = 0; i < sizeof(contents); i++)
				if(contents[i] != 0xFF) { all_erased = 0; break; }
			CHECK(all_erased, "new save contents are initialized to 0xFF");
		}
		check(f_close(&file), "close successful save read");
	}
	else
	{
		CHECK(0, "open successful save for readback");
	}

	CHECK(SavefileWrite((TCHAR*)long_save_path, sizeof(contents)) == 1,
		"successful first-time save supports a long filename");
	result = f_stat(long_save_path, &info);
	CHECK(result == FR_OK && info.fsize == sizeof(contents),
		"long-filename save is discoverable at its complete size");
}

static void fill_card_leaving_clusters(UINT keep_free)
{
	FIL file;
	FATFS *mounted_fs;
	DWORD free_clusters;
	BYTE block[SECTOR_SIZE];
	UINT written;
	DWORD i;

	memset(block, 0xA5, sizeof(block));
	check(f_getfree("0:", &free_clusters, &mounted_fs), "query free clusters");
	check(f_open(&file, "0:/FILLER.BIN", FA_WRITE | FA_CREATE_ALWAYS), "create filler file");
	for(i = 0; i < (free_clusters - keep_free) * mounted_fs->csize; i++)
	{
		FRESULT result = f_write(&file, block, sizeof(block), &written);
		if(result != FR_OK || written != sizeof(block))
		{
			fprintf(stderr, "FAIL: fill test card (FatFs result %u, wrote %u)\n",
				(unsigned)result, written);
			exit(2);
		}
	}
	check(f_close(&file), "close filler file");
}

static void test_full_card_cleanup(void)
{
	FILINFO info;
	FRESULT result;

	mount_fresh_formatted_volume();
	fill_card_leaving_clusters(20);
	CHECK(SavefileWrite("/SYSTEM/SAVER/FULL.SAV", 0x8000) == 0,
		"full card causes first-time save creation to fail");
	result = f_stat("/SYSTEM/SAVER/FULL.SAV", &info);
	CHECK(result == FR_NO_FILE, "full-card failure leaves no partial save file");
}

static void test_write_failure_cleanup(void)
{
	long clean_write_count;
	long position;
	int failures_with_partial_files = 0;
	int failures_seen = 0;

	mount_fresh_formatted_volume();
	write_calls = 0;
	fail_at = -1;
	injected_failure = 0;
	CHECK(SavefileWrite("/SYSTEM/SAVER/COUNT.SAV", 0x8000) == 1,
		"baseline save succeeds before write-failure sweep");
	clean_write_count = write_calls;

	for(position = 0; position < clean_write_count; position++)
	{
		FILINFO info;
		FRESULT result;
		UINT index;

		mount_fresh_formatted_volume();
		write_calls = 0;
		fail_at = position;
		injected_failure = 0;
		if(SavefileWrite((TCHAR*)long_save_path, 0x8000) == 0)
			failures_seen++;
		fail_at = -1;
		if(injected_failure)
		{
			index = (UINT)position;
			result = f_stat(long_save_path, &info);
			if(result == FR_OK)
			{
				FIL file;
				BYTE contents[0x8000];
				UINT bytes_read = 0;
				int complete = info.fsize == sizeof(contents) &&
					f_open(&file, long_save_path, FA_READ) == FR_OK;
				FRESULT content_result = FR_OK;
				if(complete)
				{
					content_result = f_read(&file, contents, sizeof(contents), &bytes_read);
					complete = content_result == FR_OK &&
						bytes_read == sizeof(contents);
					(void)f_close(&file);
				}
				if(complete)
				{
					size_t j;
					for(j = 0; j < sizeof(contents); j++)
						if(contents[j] != 0xFF) { complete = 0; break; }
				}
				if(!complete)
				{
					printf("  FAIL  injected disk_write #%u left an incomplete save (stat=%u size=%lu read=%u read_result=%u)\n", index, (unsigned)result, (unsigned long)info.fsize, (unsigned)bytes_read, (unsigned)content_result);
					failures_with_partial_files++;
				}
			}
			else if(result != FR_NO_FILE)
				failures_with_partial_files++;
		}
	}
	CHECK(clean_write_count > 0, "save operation performs disk writes");
	CHECK(failures_seen > 0, "write-error sweep triggers SavefileWrite failures");
	CHECK(failures_with_partial_files == 0,
		"each injected write error leaves no discoverable partial save");
}

/* Failed replacement must preserve the complete prior save. */
static void test_existing_save_overwrite_failure(void)
{
	FIL file;
	FILINFO info;
	BYTE old_contents[0x800];
	UINT written = 0;
	FRESULT result;
	u32 save_result;

	mount_fresh_formatted_volume();
	memset(old_contents, 0xA5, sizeof(old_contents));
	check(f_open(&file, long_save_path, FA_WRITE | FA_CREATE_ALWAYS),
		"create pre-existing LFN save fixture");
	check(f_write(&file, old_contents, sizeof(old_contents), &written),
		"write pre-existing save contents");
	CHECK(written == sizeof(old_contents), "pre-existing save fixture is complete");
	check(f_close(&file), "close pre-existing save fixture");
	result = f_stat(long_save_path, &info);
	CHECK(result == FR_OK && info.fsize == sizeof(old_contents),
		"precondition: old LFN save exists before overwrite");

	write_calls = 0;
	fail_at = -1;
	injected_failure = 0;
	inject_next_sram_write = 1;
	save_result = Save_savefile((TCHAR*)long_save_path, sizeof(old_contents));
	inject_next_sram_write = 0;
	fail_at = -1;
	CHECK(injected_failure, "overwrite fault is injected while writing the staged replacement");
	CHECK(save_result == 0, "failed overwrite is reported to the caller");
	result = f_stat(long_save_path, &info);
	CHECK(result == FR_OK && info.fsize == sizeof(old_contents),
		"failed overwrite keeps the original save and its full size");
	if(result == FR_OK)
	{
		BYTE actual[sizeof(old_contents)];
		UINT bytes_read = 0;
		check(f_open(&file, long_save_path, FA_READ), "open preserved old save");
		result = f_read(&file, actual, sizeof(actual), &bytes_read);
		CHECK(result == FR_OK && bytes_read == sizeof(actual) &&
			memcmp(actual, old_contents, sizeof(actual)) == 0,
			"failed overwrite preserves every byte of the previous save");
		check(f_close(&file), "close preserved old save");
	}
}

static void test_transaction_recovery(void)
{
	FIL file;
	BYTE old_contents[0x800];
	BYTE new_contents[0x800];
	BYTE actual[0x800];
	UINT written;
	UINT bytes_read;
	FRESULT result;
	const char *target = "/SYSTEM/SAVER/RECOVER.SAV";

	mount_fresh_formatted_volume();
	memset(old_contents, 0xA5, sizeof(old_contents));
	memset(new_contents, 0x5A, sizeof(new_contents));
	check(f_open(&file, target, FA_WRITE | FA_CREATE_ALWAYS), "create recovery fixture");
	check(f_write(&file, old_contents, sizeof(old_contents), &written), "write recovery fixture");
	check(f_close(&file), "close recovery fixture");
	check(f_open(&file, FILE_TXN_DATA_PATH, FA_WRITE | FA_CREATE_NEW), "create staged recovery data");
	check(f_write(&file, new_contents, sizeof(new_contents), &written), "write staged recovery data");
	check(f_close(&file), "close staged recovery data");
	file_txn_target_existed = 1;
	CHECK(FileTxn_WriteJournal(target), "write recovery journal before moving original");
	check(f_rename(target, FILE_TXN_BACKUP_PATH), "move old save to transaction backup");
	file_txn_recovered = 0;
	CHECK(FileTxn_Recover(), "recovery restores old save if promotion did not occur");
	check(f_open(&file, target, FA_READ), "open restored old save");
	result = f_read(&file, actual, sizeof(actual), &bytes_read);
	CHECK(result == FR_OK && bytes_read == sizeof(actual) &&
		memcmp(actual, old_contents, sizeof(actual)) == 0,
		"interrupted replacement returns the complete original save");
	check(f_close(&file), "close restored old save");

	/* Model FatFs having registered the destination LFN but not yet removed
	   the staged source when power is lost during rename. */
	check(f_open(&file, target, FA_WRITE | FA_CREATE_ALWAYS), "recreate old save before partial rename case");
	check(f_write(&file, old_contents, sizeof(old_contents), &written), "rewrite old save before partial rename case");
	check(f_close(&file), "close old save before partial rename case");
	check(f_open(&file, FILE_TXN_DATA_PATH, FA_WRITE | FA_CREATE_NEW), "create staged data for partial rename case");
	check(f_write(&file, new_contents, sizeof(new_contents), &written), "write staged data for partial rename case");
	check(f_close(&file), "close staged data for partial rename case");
	file_txn_target_existed = 1;
	CHECK(FileTxn_WriteJournal(target), "write journal for partial rename recovery");
	check(f_rename(target, FILE_TXN_BACKUP_PATH), "move original before partial rename recovery");
	check(f_open(&file, target, FA_WRITE | FA_CREATE_NEW), "simulate registered destination before source removal");
	check(f_write(&file, new_contents, sizeof(new_contents), &written), "write partial-rename destination fixture");
	check(f_close(&file), "close partial-rename destination fixture");
	file_txn_recovered = 0;
	CHECK(!FileTxn_Recover(), "recovery leaves ambiguous duplicate FAT entries untouched");
	CHECK(FileTxn_PathState(target) == 1 && FileTxn_PathState(FILE_TXN_BACKUP_PATH) == 1 &&
		FileTxn_PathState(FILE_TXN_DATA_PATH) == 1,
		"ambiguous recovery preserves destination, backup and staged entries");
	check(f_open(&file, FILE_TXN_BACKUP_PATH, FA_READ), "open preserved backup after ambiguous rename");
	bytes_read = 0;
	result = f_read(&file, actual, sizeof(actual), &bytes_read);
	CHECK(result == FR_OK && bytes_read == sizeof(actual) &&
		memcmp(actual, old_contents, sizeof(actual)) == 0,
		"ambiguous rename retains the complete old save in its backup");
	check(f_close(&file), "close preserved backup after ambiguous rename");

	/* A first-time save has no backup; if an interrupted rename leaves both
	   directory entries, recovery must preserve them for manual repair. */
	mount_fresh_formatted_volume();
	file_txn_recovered = 0;
	check(f_open(&file, FILE_TXN_DATA_PATH, FA_WRITE | FA_CREATE_NEW), "create first-save staged data");
	check(f_write(&file, new_contents, sizeof(new_contents), &written), "write first-save staged data");
	check(f_close(&file), "close first-save staged data");
	file_txn_target_existed = 0;
	CHECK(FileTxn_WriteJournal(target), "write first-save recovery journal");
	check(f_open(&file, target, FA_WRITE | FA_CREATE_NEW), "simulate partial first-save destination");
	check(f_write(&file, new_contents, sizeof(new_contents), &written), "write partial first-save destination");
	check(f_close(&file), "close partial first-save destination");
	file_txn_recovered = 0;
	CHECK(!FileTxn_Recover(), "recovery blocks on duplicate first-save FAT entries");
	CHECK(FileTxn_PathState(target) == 1 && FileTxn_PathState(FILE_TXN_DATA_PATH) == 1,
		"ambiguous first-save recovery preserves both staged names");

	mount_fresh_formatted_volume();
	file_txn_recovered = 0;
	check(f_open(&file, target, FA_WRITE | FA_CREATE_ALWAYS), "create old save for completed promotion case");
	check(f_write(&file, old_contents, sizeof(old_contents), &written), "write old save for completed promotion case");
	check(f_close(&file), "close old save for completed promotion case");
	check(f_open(&file, FILE_TXN_DATA_PATH, FA_WRITE | FA_CREATE_NEW), "create second staged data");
	check(f_write(&file, new_contents, sizeof(new_contents), &written), "write second staged data");
	check(f_close(&file), "close second staged data");
	file_txn_target_existed = 1;
	CHECK(FileTxn_WriteJournal(target), "write second recovery journal");
	check(f_rename(target, FILE_TXN_BACKUP_PATH), "move old save before second promotion");
	check(f_rename(FILE_TXN_DATA_PATH, target), "install complete replacement before simulated reset");
	file_txn_recovered = 0;
	CHECK(FileTxn_Recover(), "recovery finalizes a fully installed replacement");
	check(f_open(&file, target, FA_READ), "open promoted save after recovery");
	result = f_read(&file, actual, sizeof(actual), &bytes_read);
	CHECK(result == FR_OK && bytes_read == sizeof(actual) &&
		memcmp(actual, new_contents, sizeof(actual)) == 0,
		"recovery keeps the fully written replacement");
	check(f_close(&file), "close promoted save");
}

static void test_settings_transaction(void)
{
	static const char target[] = "/SYSTEM/SETTINGS.TXT";
	static const BYTE old_contents[] = "old settings\n";
	static const BYTE new_contents[] = "new settings\n";
	BYTE actual[sizeof(old_contents)];
	FIL file;
	UINT written = 0;
	UINT bytes_read = 0;
	FRESULT result;

	mount_fresh_formatted_volume();
	file_txn_recovered = 0;
	check(f_open(&file, target, FA_WRITE | FA_CREATE_ALWAYS), "create old settings fixture");
	check(f_write(&file, old_contents, sizeof(old_contents), &written), "write old settings fixture");
	check(f_close(&file), "close old settings fixture");

	CHECK(FileTxn_Begin(target, &file), "begin staged unified-settings write");
	fail_at = write_calls;
	result = f_write(&file, new_contents, sizeof(new_contents), &written);
	fail_at = -1;
	CHECK(result != FR_OK || written != sizeof(new_contents), "inject settings staging write failure");
	CHECK(!FileTxn_Commit(&file), "failed settings write is reported");
	check(f_open(&file, target, FA_READ), "open preserved settings after failed write");
	result = f_read(&file, actual, sizeof(actual), &bytes_read);
	CHECK(result == FR_OK && bytes_read == sizeof(old_contents) &&
		memcmp(actual, old_contents, sizeof(old_contents)) == 0,
		"failed settings replacement preserves previous contents");
	check(f_close(&file), "close preserved settings fixture");

	CHECK(FileTxn_Begin(target, &file), "begin successful staged settings write");
	check(f_write(&file, new_contents, sizeof(new_contents), &written), "write replacement settings");
	CHECK(FileTxn_Commit(&file), "commit complete settings replacement");
	check(f_open(&file, target, FA_READ), "open committed settings");
	bytes_read = 0;
	result = f_read(&file, actual, sizeof(new_contents), &bytes_read);
	CHECK(result == FR_OK && bytes_read == sizeof(new_contents) &&
		memcmp(actual, new_contents, sizeof(new_contents)) == 0,
		"successful settings replacement contains the complete new data");
	check(f_close(&file), "close committed settings");
}

static void test_dbcs_save_filename(void)
{
    static const char target[] = "/SYSTEM/SAVER/\x81\x5c.sav";
    static const BYTE contents[] = {1,2,3,4};
    FIL file;UINT written = 0;
    mount_fresh_formatted_volume();file_txn_recovered = 0;
    check(f_open(&file, target, FA_WRITE | FA_CREATE_ALWAYS), "FatFs accepts CP936 backslash trail byte");
    check(f_close(&file), "close CP936 direct-open fixture");
    CHECK(FileTxn_IsAllowedTarget(target), "CP936 trail byte is a filename character, not a separator");
    if(!FileTxn_Begin(target, &file)) {
        CHECK(0, "begin transaction for valid CP936 save filename");return;
    }
    check(f_write(&file, contents, sizeof contents, &written), "write CP936 staged save");
    CHECK(written == sizeof contents && FileTxn_Commit(&file), "commit CP936 save transaction");
    BYTE actual[sizeof contents];UINT received = 0;
    check(f_open(&file, target, FA_READ), "read CP936 save target");
    check(f_read(&file, actual, sizeof actual, &received), "read complete CP936 save contents");
    CHECK(received == sizeof actual && !memcmp(actual, contents, sizeof actual), "CP936 save contents retained");
    check(f_close(&file), "close CP936 save target");
}

int main(void)
{
    CHECK(!FileTxn_IsAllowedTarget("/SYSTEM/SAVER/\x81\x5c\\nested.sav"), "real separator after CP936 pair is rejected");
    CHECK(!FileTxn_IsAllowedTarget("/SYSTEM/SAVER/\x81/nested.sav"), "forward separator after invalid DBCS lead is rejected");
    CHECK(!FileTxn_IsAllowedTarget(NULL) && !FileTxn_IsAllowedTarget(""), "empty transaction target is rejected");

	CHECK(FileTxn_IsAllowedTarget("/system/saver/lower.sav"), "lowercase FAT save directory is accepted");
	CHECK(FileTxn_IsAllowedTarget("0:/System/Settings.txt"), "mixed-case settings path is accepted");
	CHECK(!FileTxn_IsAllowedTarget("/SYSTEM/SAVER/..\\..\\game.gba"), "backslash directory traversal is rejected");
	CHECK(!FileTxn_IsAllowedTarget("/SYSTEM/SAVER/sub/game.sav"), "nested save target is rejected");

	printf("[save-file creation, overwrite, and failure handling]\n");
	format_volume();
	test_new_save_creation();
	test_full_card_cleanup();
	test_write_failure_cleanup();
	test_existing_save_overwrite_failure();
	test_transaction_recovery();
	test_settings_transaction();
	test_dbcs_save_filename();
	printf("%s (%d failure%s)\n", failures ? "FAILED" : "PASSED",
		failures, failures == 1 ? "" : "s");
	return failures ? 1 : 0;
}
