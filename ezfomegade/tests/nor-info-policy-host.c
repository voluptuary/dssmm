#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nor_info_policy.h"

static void check(int condition, const char *message)
{
	if(!condition)
	{
		fprintf(stderr, "FAIL: %s\n", message);
		exit(1);
	}
}

int main(void)
{
	/* Sizes used by the firmware: 0x200-byte SET/SAVE records and the 8448-byte NOR catalog. */
	static const unsigned sizes[] = { 0x200u, 8448u };
	unsigned s;

	for(s = 0; s < sizeof(sizes) / sizeof(sizes[0]); s++)
	{
		unsigned bytes = sizes[s], words = bytes / 2u, i;
		uint16_t *flash = malloc(bytes + 64u);   /* the record plus unrelated sector contents after it */
		uint16_t *data = malloc(bytes);
		unsigned probes[3];

		check(flash && data, "allocation");
		for(i = 0; i < words; i++) { data[i] = (uint16_t)(0x1234u + i * 7u); flash[i] = data[i]; }
		memset(flash + words, 0xA5, 64);

		check(NorInfo_RegionMatches(flash, data, bytes), "identical content matches (write may be skipped)");

		probes[0] = 0; probes[1] = words / 2u; probes[2] = words - 1u;
		for(i = 0; i < 3; i++)
		{
			uint16_t saved = flash[probes[i]];
			flash[probes[i]] ^= 0x0001u;                    /* a single flipped bit */
			check(!NorInfo_RegionMatches(flash, data, bytes), "a one-bit difference must force the write");
			flash[probes[i]] = saved;
		}
		check(NorInfo_RegionMatches(flash, data, bytes), "restored content matches again");

		/* An erased record (all 0xFF) must never be mistaken for the new content. */
		memset(flash, 0xFF, bytes);
		check(!NorInfo_RegionMatches(flash, data, bytes), "erased flash must force the write");

		/* A cleared catalog (all zero) already in flash needs no rewrite. */
		memset(flash, 0x00, bytes); memset(data, 0x00, bytes);
		check(NorInfo_RegionMatches(flash, data, bytes), "all-zero catalog already stored needs no rewrite");

		/* Bytes after the record are not part of the comparison. */
		memset(flash + words, 0x5A, 64);
		check(NorInfo_RegionMatches(flash, data, bytes), "data after the record is ignored");
		free(flash); free(data);
	}

	{
		uint16_t a[2] = { 1, 2 }, b[2] = { 1, 2 };
		check(!NorInfo_RegionMatches(a, b, 3), "odd length never matches");
		check(NorInfo_RegionMatches(a, b, 0), "zero length trivially matches (nothing to write)");
		check(NorInfo_RegionMatches(a, b, 2), "single word compares only that word");
		b[0] = 9;
		check(!NorInfo_RegionMatches(a, b, 2), "single differing word detected");
	}

	puts("NOR info policy host test passed.");
	return 0;
}
