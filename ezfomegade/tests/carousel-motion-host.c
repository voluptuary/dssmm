#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t u32;
typedef struct { int x, y, w, h; } LauncherMotionRect;
typedef struct { unsigned id; u32 icon; } LauncherMotionImage;
typedef struct { u32 absolute_index; } LauncherThumbCache;
static unsigned char code[0x8000];
static const unsigned char *__start_motion_code = code, *__stop_motion_code;
static LauncherThumbCache launcher_cache_prev = {0}, launcher_cache_selected = {1}, launcher_cache_next = {2};
static u32 launcher_carousel_animations = 1, launcher_motion_finished = UINT32_MAX, launcher_cache_center_index = 1;
static struct { LauncherMotionImage images[4]; LauncherMotionRect current[4]; } work;
#define LAUNCHER_MOTION_WORK (&work)
static unsigned copies, image_reads, background_calls, blits, presents, failures;
static u32 copy_size;
static uintptr_t copy_dest;
static int region_valid = 1;

static void expect(int condition, const char *message)
{
    if(!condition) { fprintf(stderr, "FAIL: %s\n", message); failures++; }
}
static void dmaCopy(const void *source, void *dest, u32 size)
{
    expect(source == code, "installer uses section source");
    copies++; copy_dest = (uintptr_t)dest; copy_size = size;
}
static u32 Launcher_ActiveViewMode(void) { return 1; }
static LauncherMotionRect Launcher_MotionRegion(u32 view)
{
    (void)view; return (LauncherMotionRect){0, 19, 240, 100};
}
static u32 Launcher_MotionRegionValid(LauncherMotionRect r, u32 view)
{
    (void)r; (void)view; return region_valid;
}
static void UIAudio_StopForSharedBufferUse(void) {}
static void Launcher_MotionPrepareBackground(LauncherMotionRect r) { (void)r; }
static LauncherMotionImage Launcher_MotionImage(const LauncherThumbCache *cache, u32 index)
{
    expect(cache->absolute_index == index, "image resolves matching cache entry");
    image_reads++; return (LauncherMotionImage){index, index == 2};
}
static LauncherMotionRect Launcher_MotionSlot(int slot, u32 view)
{
    (void)view; return (LauncherMotionRect){(slot + 1) * 80, 20, 80, 80};
}
static LauncherMotionRect Launcher_MotionImageRect(LauncherMotionRect r, u32 icon, int slot)
{
    (void)slot;
    if(icon) { r.w = 16; r.h = 14; }
    return r;
}
static void Launcher_MotionBackground(LauncherMotionRect clip, const LauncherMotionRect *r,
    const LauncherMotionImage *images, int count)
{
    (void)clip;
    expect(count == 3 && images[0].id == 0 && images[1].id == 1 && images[2].id == 2,
        "background uses all three prepared images");
    expect(r[2].w == 16 && r[1].w == 80, "background uses artwork and icon geometry");
    background_calls++;
}
static void Launcher_MotionBlit(LauncherMotionRect clip, LauncherMotionRect r, LauncherMotionImage image, u32 selected)
{
    static const unsigned order[] = {0, 2, 1};
    (void)clip;
    expect(blits < 3 && image.id == order[blits], "selected image draws after both side images");
    expect(selected == (image.id == 1), "selected border follows center image");
    expect(r.x == (int)image.id * 80 && r.w == (image.icon ? 16 : 80), "blit preserves prepared geometry");
    blits++;
}
static void Launcher_MotionPresent(LauncherMotionRect r) { (void)r; presents++; }

#include MOTION_FILE

int main(void)
{
    static const u32 sizes[] = {4, 2796, 0x3E00, 0, 2, 0x3E02, 0x3E04, 0x8000};
    unsigned i;
    for(i = 0; i < sizeof(sizes) / sizeof(*sizes); i++)
    {
        int valid = i < 3;
        copies = 0; __stop_motion_code = code + sizes[i];
        expect(Launcher_MotionInstallCode() == (u32)valid, "installer rejects empty, unaligned, or oversized code");
        expect(copies == (unsigned)valid, "rejected installer never starts DMA");
        if(valid) expect(copy_dest == 0x06014200 && copy_size == sizes[i], "copy stays inside sprite VRAM");
    }
    __stop_motion_code = code + 2796;
    expect(Launcher_DrawMotionRest() == 1, "rest frame renders");
    expect(image_reads == 3, "rest frame resolves and scans each image once");
    expect(blits == 3 && background_calls == 1 && presents == 1, "complete rest frame draws once");
    region_valid = 0;
    expect(Launcher_DrawMotionRest() == 0, "invalid layout falls back");
    launcher_carousel_animations = 0;
    expect(Launcher_DrawMotionRest() == 0, "disabled animation falls back");
    expect(blits == 3 && presents == 1, "fallbacks do not present partial frames");
    launcher_carousel_animations = 1;
    launcher_motion_finished = launcher_cache_center_index;
    expect(Launcher_DrawMotionRest() == 1 && launcher_motion_finished == UINT32_MAX,
        "completed animation consumes its frame without redrawing");
    if(failures) return EXIT_FAILURE;
    puts("Carousel install bounds and rest-frame scenarios passed.");
    return EXIT_SUCCESS;
}
