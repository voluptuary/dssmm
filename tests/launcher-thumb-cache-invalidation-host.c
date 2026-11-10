#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int u32;

typedef struct
{
	u32 valid;
	u32 absolute_index;
	u32 has_thumbnail;
	u32 custom_thumbnail;
	unsigned char *thumb_data;
} LauncherThumbCache;

static LauncherThumbCache launcher_cache_prev;
static LauncherThumbCache launcher_cache_selected;
static LauncherThumbCache launcher_cache_next;
static u32 launcher_cache_center_index;

#include CACHE_INVALIDATION_FILE

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
	unsigned char prev_data[4] = {1, 2, 3, 4};
	unsigned char selected_data[4] = {5, 6, 7, 8};
	unsigned char next_data[4] = {9, 10, 11, 12};
	unsigned char prev_snapshot[sizeof(prev_data)];
	unsigned char selected_snapshot[sizeof(selected_data)];
	unsigned char next_snapshot[sizeof(next_data)];

	launcher_cache_prev = (LauncherThumbCache){1, 20, 1, 1, prev_data};
	launcher_cache_selected = (LauncherThumbCache){1, 21, 1, 1, selected_data};
	launcher_cache_next = (LauncherThumbCache){1, 22, 1, 1, next_data};
	launcher_cache_center_index = 21;
	for(size_t i = 0; i < sizeof(prev_data); ++i)
	{
		prev_snapshot[i] = prev_data[i];
		selected_snapshot[i] = selected_data[i];
		next_snapshot[i] = next_data[i];
	}

	Launcher_InvalidateThumbCacheEntries();

	expect(launcher_cache_center_index == 0xFFFFFFFF,
		"invalidating records clears the cached center identity");
	expect(!launcher_cache_prev.valid && !launcher_cache_selected.valid &&
		!launcher_cache_next.valid,
		"invalidating records marks all three slots unusable");
	expect(launcher_cache_prev.absolute_index == 0xFFFFFFFF &&
		launcher_cache_selected.absolute_index == 0xFFFFFFFF &&
		launcher_cache_next.absolute_index == 0xFFFFFFFF,
		"invalidating records clears all cached indices");
	expect(!launcher_cache_prev.has_thumbnail && !launcher_cache_selected.has_thumbnail &&
		!launcher_cache_next.has_thumbnail && !launcher_cache_prev.custom_thumbnail &&
		!launcher_cache_selected.custom_thumbnail && !launcher_cache_next.custom_thumbnail,
		"invalidating records clears thumbnail metadata");
	expect(launcher_cache_prev.thumb_data == prev_data &&
		launcher_cache_selected.thumb_data == selected_data &&
		launcher_cache_next.thumb_data == next_data,
		"metadata invalidation keeps the cache buffer pointers");
	expect(memcmp(prev_data, prev_snapshot, sizeof(prev_data)) == 0 &&
		memcmp(selected_data, selected_snapshot, sizeof(selected_data)) == 0 &&
		memcmp(next_data, next_snapshot, sizeof(next_data)) == 0,
		"metadata invalidation does not clear or copy image payloads");

	puts("Thumbnail cache metadata invalidation scenarios passed.");
	return 0;
}
