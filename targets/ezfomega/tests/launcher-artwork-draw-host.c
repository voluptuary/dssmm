/*
 * Differential test for the artwork row drawing routines. The routines are extracted verbatim from a reference
 * version and the working tree; their VRAM output and DMA row-copy count must match. The span helper is also
 * checked against a plain per-pixel scan.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;
typedef volatile u16 vu16;
#include SOURCE_CONSTANTS

static u16 g_vram[240 * 160 + 64] __attribute__((aligned(8)));
static unsigned g_dma_calls;
#define HOST_VRAM_BASE ((unsigned long)g_vram)
#define VRAM HOST_VRAM_BASE

static void dmaCopy(const void *source, void *dest, u32 size) { memcpy(dest, source, size); g_dma_calls++; }
static u32 Launcher_RoundedThumbPixelVisible(int dst_x, int dst_y, int w, int h)
{
	int dx = dst_x < w - 1 - dst_x ? dst_x : w - 1 - dst_x;
	int dy = dst_y < h - 1 - dst_y ? dst_y : h - 1 - dst_y;
	return !(dx < 4 && dy < 4 && dx + dy < 3);
}

#include FUNCS_FILE

static u32 rng_state = 0x1234567u;
static u32 rnd(void) { rng_state ^= rng_state << 13; rng_state ^= rng_state >> 17; rng_state ^= rng_state << 5; return rng_state; }
static u16 pixel(void) { return (u16)(rnd() & 0x7FFF); }
static u32 fnv(u32 h, u32 v) { return (h ^ v) * 16777619u; }

static u16 src_store[(80 + 2) * 140 + 16] __attribute__((aligned(8)));

#ifdef HAVE_HELPER
static int exact_scan(const u16 *p, int n)
{
	int i;
	for(i = 0; i < n; i++)
		if(p[i] == LAUNCHER_THUMB_TRANSPARENT_SENTINEL)
			return 1;
	return 0;
}

static void fuzz_helper(void)
{
	static u16 buf[200] __attribute__((aligned(8)));
	unsigned long cases = 0, bad = 0;
	int len, off, mode, rep, i;

	for(len = -1; len <= 140; len++)
		for(off = 0; off < 4; off++)
			for(mode = 0; mode < 9; mode++)
				for(rep = 0; rep < 150; rep++)
				{
					const u16 *p = buf + off;
					int n = len < 0 ? 0 : len;
					for(i = 0; i < 160; i++)
						buf[off + i] = pixel();
					if(mode == 1 && n) buf[off + rnd() % n] = 0x8000;
					if(mode == 2 && n) buf[off + (rnd() & 1 ? 0 : n - 1)] = 0x8000;
					if(mode == 3 && n) for(i = 0; i < 3; i++) buf[off + rnd() % n] = (u16)(0x8000 | (pixel() | 1));
					if(mode == 4 && n) for(i = 0; i < n; i++) buf[off + i] = 0x8000;
					if(mode == 5 && n) for(i = 0; i < n; i++) buf[off + i] = 0;
					if(mode == 6 && n) { buf[off + rnd() % n] = 0xFFFF; buf[off + rnd() % n] = 0x8001; }
					if(mode == 7 && n) buf[off] = 0x8000;
					if(mode == 8 && n) buf[off + n - 1] = 0x8000;
					cases++;
					if(Launcher_SpanHasTransparent(p, len) != exact_scan(p, n))
						bad++;
				}
	if(Launcher_SpanHasTransparent(NULL, 1) != 0 ||
		Launcher_SpanHasTransparent(buf, 0) != 0 ||
		Launcher_SpanHasTransparent(buf, -1) != 0)
		bad++;
	printf("span|cases=%lu|mismatches=%lu\n", cases, bad);
}
#endif

int main(void)
{
	u32 acc = 2166136261u, hash;
	int c, fn, i, w, h, stride, x, y, off, mode, r, col;
	long total = 0;

#ifdef HAVE_HELPER
	fuzz_helper();
#endif
	rng_state = 0x1234567u;
	for(c = 0; c < 120000; c++)
	{
		fn = c & 1; mode = (c >> 1) % 8;
		w = 1 + (int)(rnd() % 120); h = 1 + (int)(rnd() % 80); stride = w + (int)(rnd() % 10);
		off = (int)(rnd() % 4); x = (int)(rnd() % 240); y = (int)(rnd() % 160);
		for(r = 0; r < h; r++)
			for(col = 0; col < stride; col++)
			{
				u16 px = pixel();
				if(mode == 1 && rnd() % 50 == 0) px = 0x8000;
				if(mode == 2 && rnd() % 500 == 0) px = 0x8000;
				if(mode == 3 && col == 0 && rnd() % 2) px = 0x8000;
				if(mode == 4 && col == w - 1 && rnd() % 2) px = 0x8000;
				if(mode == 5 && rnd() % 30 == 0) px = (u16)(0x8000 | (pixel() | 1));
				if(mode == 6) px = 0x8000;
				if(mode == 7 && r % 3 == 0 && rnd() % 20 == 0) px = 0x8000;
				src_store[off + r * stride + col] = px;
			}
		for(i = 0; i < 240 * 160; i++)
			g_vram[i] = (u16)(0x4000 | ((i * 131) & 0x3FFF));
		g_dma_calls = 0;
		if(fn)
			Launcher_DrawPicClipStride(src_store + off, stride, x, y, w, h);
		else
			Launcher_DrawPreparedThumbMasked(src_store + off, stride, x, y, w, h);
		hash = 2166136261u;
		for(i = 0; i < 240 * 160; i++)
			hash = fnv(hash, g_vram[i]);
		acc = fnv(fnv(acc, hash), g_dma_calls); total += g_dma_calls;
		if(c % 10000 == 9999)
			printf("draw|%d|%08x\n", c + 1, acc);
	}
	printf("draw|final|%08x|dma_row_copies=%ld\n", acc, total);
	return 0;
}
