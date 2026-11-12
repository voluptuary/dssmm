#ifndef FAT_TABLE_BUILDER_H
#define FAT_TABLE_BUILDER_H

#include <stddef.h>
#include <stdint.h>

#define FAT_TABLE_SIZE_BYTES 0x400u
#define FAT_TABLE_GAME_PARAMETER_OFFSET_BYTES 0x1F0u
#define FAT_TABLE_RTS_OFFSET_BYTES 0x300u

static inline uint32_t *FatTableLimit(uint32_t *table, int game_rom)
{
	size_t limit_bytes = game_rom
		? FAT_TABLE_GAME_PARAMETER_OFFSET_BYTES
		: FAT_TABLE_SIZE_BYTES;

	return table + limit_bytes / sizeof(*table);
}

static inline int FatTableAppendPair(uint32_t **cursor, uint32_t *limit,
	uint32_t first, uint32_t second)
{
	if(*cursor > limit || (size_t)(limit - *cursor) < 2u)
		return 0;

	(*cursor)[0] = first;
	(*cursor)[1] = second;
	*cursor += 2;
	return 1;
}

#endif
