#ifndef PSRAM_LOAD_POLICY_H
#define PSRAM_LOAD_POLICY_H

#include <stdint.h>
#include <string.h>

#include "ez_define.h"
#include "ff.h"

static inline uint32_t PSRAM_LoadChunkSize(uint32_t remaining)
{
	return remaining > MAX_pReadCache_size ? MAX_pReadCache_size : remaining;
}

static inline int PSRAM_LoadRangeFits(uint32_t offset, uint32_t size)
{
	return offset <= PSRAM_CAPACITY_BYTES &&
		size <= PSRAM_CAPACITY_BYTES - offset;
}

static inline int PSRAM_LoadAlignUp(uint32_t value, uint32_t alignment,
	uint32_t *aligned)
{
	uint32_t mask;
	if(!aligned || !alignment || (alignment & (alignment - 1)))
		return 0;
	mask = alignment - 1;
	if(value > UINT32_MAX - mask)
		return 0;
	*aligned = (value + mask) & ~mask;
	return 1;
}

static inline int PSRAM_LoadPaddedSize(uint32_t file_size, uint32_t *padded_size)
{
	if(!file_size || !padded_size ||
		!PSRAM_LoadAlignUp(file_size, MAX_pReadCache_size, padded_size))
		return 0;
	return 1;
}

typedef struct {
	uint16_t page;
	uint32_t window_offset;
	uint32_t size;
} PSRAM_LoadWindowPlan;

/* Plan one DMA segment in a banked PSRAM aperture. page_step is the selector
   increment for each aperture window (0x1000 for 8 MiB, 0x0800 for 4 MiB). */
static inline int PSRAM_LoadPlanWindow(uint32_t address, uint32_t remaining,
	uint32_t window_size, uint32_t page_step, PSRAM_LoadWindowPlan *plan)
{
	uint32_t page;
	uint32_t window_number;
	uint32_t window_offset;
	uint32_t available;

	if(!plan || !remaining || !window_size || !page_step ||
		!PSRAM_LoadRangeFits(address, remaining))
		return 0;
	window_number = address / window_size;
	if(window_number > UINT16_MAX / page_step)
		return 0;
	page = window_number * page_step;
	if(page > UINT16_MAX)
		return 0;
	window_offset = address % window_size;
	available = window_size - window_offset;
	plan->page = (uint16_t)page;
	plan->window_offset = window_offset;
	plan->size = remaining < available ? remaining : available;
	return plan->size != 0;
}

/* Read only bytes belonging to the file. A short/error read is never accepted;
   the unused tail of the final pool-sized block has deterministic padding. */
static inline int PSRAM_ReadFileChunk(FIL *file, uint8_t *buffer,
	uint32_t remaining, uint8_t padding, uint32_t *bytes_read)
{
	UINT actual = 0;
	uint32_t expected;

	if(bytes_read)
		*bytes_read = 0;
	if(!file || !buffer || !bytes_read)
		return 0;
	expected = PSRAM_LoadChunkSize(remaining);
	if(!expected)
		return 0;
	if(expected < MAX_pReadCache_size)
		memset(buffer, padding, MAX_pReadCache_size);
	if(f_read(file, buffer, (UINT)expected, &actual) != FR_OK || actual != expected)
		return 0;
	*bytes_read = expected;
	return 1;
}

#endif
