#ifndef NOR_INFO_POLICY_H
#define NOR_INFO_POLICY_H

#include <stdint.h>

/*
 * Returns nonzero when the first `bytes` bytes of a flash info record already hold
 * exactly `data`. Save_info() erases a whole 64 KiB sector before it reprograms the
 * record, so a rewrite of identical content only adds risk (a failure after the erase
 * leaves the record blank) and flash wear. Callers must read while the chip is in
 * read-array mode. An odd length is never treated as a match.
 */
static __inline__ int NorInfo_RegionMatches(const volatile uint16_t *flash,
	const uint16_t *data, uint32_t bytes)
{
	uint32_t i;
	uint32_t words = bytes / 2u;

	if((bytes & 1u) != 0u)
		return 0;
	for(i = 0; i < words; i++)
	{
		if(flash[i] != data[i])
			return 0;
	}
	return 1;
}

#endif
