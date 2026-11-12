#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "fat_table_builder.h"

typedef uint32_t u32;
typedef uint32_t DWORD;
typedef char TCHAR;

typedef struct {
	DWORD sclust;
} FFOBJID;

typedef struct {
	FFOBJID obj;
} FIL;

typedef struct {
	unsigned fs_type;
	u32 csize;
} FATFS;

#define FR_OK 0
#define FA_READ 1
#define FS_FAT16 1
#define FAT_table_size FAT_TABLE_SIZE_BYTES
#define FAT_table_RTS_offset FAT_TABLE_RTS_OFFSET_BYTES

static FATFS EZcardFs;
static u32 FAT_table_buffer[FAT_table_size / sizeof(u32) + 1u];
static DWORD chain[128];
static size_t chain_length;
static size_t chain_index;
static unsigned close_count;

static int f_open(FIL *file, const TCHAR *filename, unsigned mode)
{
	(void)filename;
	(void)mode;
	file->obj.sclust = 2;
	chain_index = 0;
	return FR_OK;
}

static int f_close(FIL *file)
{
	(void)file;
	close_count++;
	return FR_OK;
}

static DWORD Get_NextCluster(FFOBJID *obj, DWORD cluster)
{
	(void)obj;
	(void)cluster;
	if(chain_index >= chain_length)
		return 0xFFFFu;
	return chain[chain_index++];
}

static u32 ClustToSect(FATFS *fs, DWORD cluster)
{
	(void)fs;
	return cluster + 1000u;
}

#ifndef CHECK_GAME_RTS_FAT_IMPL
#error CHECK_GAME_RTS_FAT_IMPL must name the extracted firmware function
#endif
#include CHECK_GAME_RTS_FAT_IMPL

static void check(int condition, const char *message)
{
	if(!condition)
	{
		fprintf(stderr, "FAIL: %s\n", message);
		exit(1);
	}
}

static void test_region(int game_rom, size_t start_word, size_t pair_capacity)
{
	uint32_t table[FAT_TABLE_SIZE_BYTES / sizeof(uint32_t) + 1u];
	uint32_t *cursor;
	uint32_t *start = table + start_word;
	uint32_t *limit = FatTableLimit(table, game_rom);
	size_t i;

	for(i = 0; i < sizeof(table) / sizeof(table[0]); i++)
		table[i] = 0xA5A5A5A5u;

	cursor = start;
	check((size_t)(limit - start) == pair_capacity * 2u, "region capacity");

	for(i = 0; i < pair_capacity; i++)
		check(FatTableAppendPair(&cursor, limit, (uint32_t)i, (uint32_t)(i + 1u)),
			"pair fits before the region limit");

	check(cursor == limit, "exact-capacity writes end at the region limit");
	check(!FatTableAppendPair(&cursor, limit, 0xDEADBEEFu, 0xCAFEBABEu),
		"overflowing pair is rejected");
	check(cursor == limit, "rejected pair leaves the cursor unchanged");
	check(*limit == 0xA5A5A5A5u, "rejected pair does not overwrite the next region");
	check(table[FAT_TABLE_SIZE_BYTES / sizeof(uint32_t)] == 0xA5A5A5A5u,
		"no write crosses the complete table boundary");
}

static void reset_fixture(void)
{
	size_t i;
	for(i = 0; i < sizeof(FAT_table_buffer) / sizeof(FAT_table_buffer[0]); i++)
		FAT_table_buffer[i] = 0xA5A5A5A5u;
	EZcardFs.fs_type = FS_FAT16;
	EZcardFs.csize = 1;
	chain_length = 0;
	chain_index = 0;
	close_count = 0;
}

static void test_valid_fragmented_chain(void)
{
	reset_fixture();
	chain[0] = 3;
	chain[1] = 20;
	chain[2] = 0xFFFFu;
	chain_length = 3;

	check(Check_game_RTS_FAT("GAME.GBA", 1) == 0,
		"valid fragmented ROM chain is accepted");
	check(close_count == 1, "valid chain closes its file");
	check(FAT_table_buffer[0] == 0 && FAT_table_buffer[1] == 1002u,
		"first ROM fragment points to its initial sector");
	check(FAT_table_buffer[2] == 2 && FAT_table_buffer[3] == 1020u,
		"next ROM fragment uses the cumulative sector offset");
	check(FAT_table_buffer[4] == 0xFFFFFFFFu && FAT_table_buffer[5] == 0,
		"ROM chain terminator is preserved");
}

static void test_maximum_fragmented_chain(int game_rom, size_t fragment_capacity)
{
	size_t i;
	size_t table_end_word = game_rom
		? FAT_TABLE_GAME_PARAMETER_OFFSET_BYTES / sizeof(u32)
		: FAT_TABLE_SIZE_BYTES / sizeof(u32);
	u32 mode = game_rom ? 1u : 3u;

	reset_fixture();
	for(i = 0; i + 1u < fragment_capacity; i++)
		chain[i] = (DWORD)(10u + i * 2u);
	chain[fragment_capacity - 1u] = 0xFFFFu;
	chain_length = fragment_capacity;

	check(Check_game_RTS_FAT("MAXFRAG.RTS", mode) == 0,
		"maximum representable fragmented chain is accepted");
	check(close_count == 1, "maximum chain closes its file");
	check(FAT_table_buffer[table_end_word - 2u] == 0xFFFFFFFFu &&
		FAT_table_buffer[table_end_word - 1u] == 0,
		"maximum chain retains its terminator at the final table pair");
	check(FAT_table_buffer[table_end_word] == 0xA5A5A5A5u,
		"maximum chain leaves the following region untouched");
}

static void test_rom_chain_overflow(void)
{
	size_t i;
	reset_fixture();
	for(i = 0; i < 61; i++)
		chain[i] = (DWORD)(10u + i * 2u);
	chain[61] = 0xFFFFu;
	chain_length = 62;

	check(Check_game_RTS_FAT("FRAGMENTED.GBA", 1) == 0xFFFFFFFFu,
		"ROM chain exceeding the FPGA table is rejected");
	check(close_count == 1, "overflowing ROM chain closes its file");
	check(FAT_table_buffer[FAT_TABLE_GAME_PARAMETER_OFFSET_BYTES / sizeof(u32)]
		== 0xA5A5A5A5u, "ROM overflow does not overwrite the parameter area");
}

static void test_rts_chain_overflow(void)
{
	size_t i;
	reset_fixture();
	for(i = 0; i < 31; i++)
		chain[i] = (DWORD)(10u + i * 2u);
	chain[31] = 0xFFFFu;
	chain_length = 32;

	check(Check_game_RTS_FAT("FRAGMENTED.RTS", 3) == 0xFFFFFFFFu,
		"RTS chain exceeding its FPGA table region is rejected");
	check(close_count == 1, "overflowing RTS chain closes its file");
	check(FAT_table_buffer[FAT_TABLE_SIZE_BYTES / sizeof(u32)]
		== 0xA5A5A5A5u, "RTS overflow does not write past the table");
}

int main(void)
{
	test_region(1, 0, FAT_TABLE_GAME_PARAMETER_OFFSET_BYTES / sizeof(uint32_t) / 2u);
	test_region(0, FAT_TABLE_RTS_OFFSET_BYTES / sizeof(uint32_t),
		(FAT_TABLE_SIZE_BYTES - FAT_TABLE_RTS_OFFSET_BYTES) / sizeof(uint32_t) / 2u);
	test_valid_fragmented_chain();
	test_maximum_fragmented_chain(1,
		FAT_TABLE_GAME_PARAMETER_OFFSET_BYTES / sizeof(u32) / 2u - 1u);
	test_maximum_fragmented_chain(0,
		(FAT_TABLE_SIZE_BYTES - FAT_TABLE_RTS_OFFSET_BYTES) / sizeof(u32) / 2u - 1u);
	test_rom_chain_overflow();
	test_rts_chain_overflow();
	puts("FAT table builder host test passed.");
	return 0;
}
