#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "psram_load_policy.h"

static FRESULT stub_read_result = FR_OK;
static UINT stub_read_count;
static BYTE stub_read_value = 0x5A;
static UINT stub_read_request;

FRESULT f_read(FIL *file, void *buffer, UINT requested, UINT *actual)
{
	(void)file;
	stub_read_request = requested;
	*actual = stub_read_count;
	if(stub_read_result == FR_OK)
		memset(buffer, stub_read_value, stub_read_count);
	return stub_read_result;
}

static void expect(int condition, const char *message)
{
	if(!condition)
	{
		fprintf(stderr, "FAIL: %s\n", message);
		exit(1);
	}
}

int main(void)
{
	uint32_t value;
	uint32_t aligned;
	PSRAM_LoadWindowPlan plan;
	FIL file;
	BYTE buffer[MAX_pReadCache_size];

	expect(PSRAM_LoadRangeFits(0, PSRAM_CAPACITY_BYTES),
		"full PSRAM capacity is a valid range");
	expect(!PSRAM_LoadRangeFits(PSRAM_CAPACITY_BYTES, 1),
		"range cannot begin past PSRAM capacity");
	expect(!PSRAM_LoadRangeFits(UINT32_MAX, 2),
		"range arithmetic cannot wrap around");

	expect(PSRAM_LoadAlignUp(0x1234, 4, &aligned) && aligned == 0x1234,
		"already aligned value remains unchanged");
	expect(PSRAM_LoadAlignUp(0x1235, 4, &aligned) && aligned == 0x1238,
		"unaligned value rounds up");
	expect(!PSRAM_LoadAlignUp(1, 3, &aligned),
		"non-power-of-two alignment is rejected");
	expect(!PSRAM_LoadAlignUp(UINT32_MAX, 4, &aligned),
		"alignment overflow is rejected");

	expect(PSRAM_LoadPaddedSize(1, &value) && value == MAX_pReadCache_size,
		"small file reserves one full DMA block");
	expect(PSRAM_LoadPaddedSize(MAX_pReadCache_size, &value) &&
		value == MAX_pReadCache_size,
		"full DMA block does not gain an extra block");
	expect(PSRAM_LoadPaddedSize(MAX_pReadCache_size + 1, &value) &&
		value == 2 * MAX_pReadCache_size,
		"partial second DMA block is included in padded span");
	expect(PSRAM_LoadPaddedSize(PSRAM_CAPACITY_BYTES, &value) &&
		value == PSRAM_CAPACITY_BYTES,
		"capacity-sized image remains representable");
	expect(!PSRAM_LoadPaddedSize(0, &value), "empty images are rejected");

	expect(PSRAM_LoadPlanWindow(PSRAM_WINDOW_BYTES - 0x10000, 0x20000,
		PSRAM_WINDOW_BYTES, 0x1000, &plan),
		"8 MiB bank-crossing DMA can be planned");
	expect(plan.page == 0 && plan.window_offset == PSRAM_WINDOW_BYTES - 0x10000 &&
		plan.size == 0x10000,
		"first 8 MiB bank segment ends at the aperture boundary");
	expect(PSRAM_LoadPlanWindow(PSRAM_WINDOW_BYTES, 0x10000,
		PSRAM_WINDOW_BYTES, 0x1000, &plan) && plan.page == 0x1000 &&
		plan.window_offset == 0 && plan.size == 0x10000,
		"second 8 MiB bank segment selects the next page");
	expect(PSRAM_LoadPlanWindow(0x400000 - 0x10000, 0x20000,
		0x400000, 0x800, &plan) && plan.page == 0 && plan.size == 0x10000,
		"legacy 4 MiB plugin aperture is split at its boundary");
	expect(PSRAM_LoadPlanWindow(0x400000, 0x10000,
		0x400000, 0x800, &plan) && plan.page == 0x800 &&
		plan.window_offset == 0,
		"legacy plugin page selector advances by 0x800");
	expect(!PSRAM_LoadPlanWindow(PSRAM_CAPACITY_BYTES - 0x10000, 0x20000,
		PSRAM_WINDOW_BYTES, 0x1000, &plan),
		"DMA extending past physical PSRAM is rejected");
	expect(!PSRAM_LoadPlanWindow(0, 1, 0, 0x1000, &plan),
		"invalid aperture parameters are rejected");
	expect(!PSRAM_LoadPlanWindow(PSRAM_CAPACITY_BYTES - 1, 1, 1, UINT32_MAX, &plan),
		"page selector arithmetic cannot overflow");

	memset(buffer, 0xCC, sizeof(buffer));
	stub_read_result = FR_OK;
	stub_read_count = 37;
	stub_read_value = 0x5A;
	expect(PSRAM_ReadFileChunk(&file, buffer, 37, 0xA5, &value) && value == 37,
		"exact short final chunk succeeds");
	expect(stub_read_request == 37 && buffer[0] == 0x5A && buffer[36] == 0x5A &&
		buffer[37] == 0xA5 && buffer[sizeof(buffer) - 1] == 0xA5,
		"final chunk reads only file bytes and pads the unused DMA tail");

	stub_read_count = MAX_pReadCache_size - 1;
	expect(!PSRAM_ReadFileChunk(&file, buffer, MAX_pReadCache_size, 0, &value),
		"short read of a full block fails");
	stub_read_result = FR_DISK_ERR;
	stub_read_count = 37;
	expect(!PSRAM_ReadFileChunk(&file, buffer, 37, 0, &value),
		"FatFs read error fails the chunk");
	expect(value == 0, "failed reads do not report usable bytes");

	puts("PSRAM load policy scenarios passed.");
	return 0;
}
