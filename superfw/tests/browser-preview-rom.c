#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "browser_carousel.h"
#include "browser_motion.h"
#include "artwork.h"
#include "browser_fixture_assets.h"
#include "fonts/font_render.h"
#include "browser_preview_text.h"
static const uint32_t empty_font[2] = {0, 0};
void *font_base_addr = (void *)empty_font;
static unsigned text_animation;

#define REG_DISPCNT (*(volatile uint16_t *)0x04000000)
#define REG_VCOUNT (*(volatile uint16_t *)0x04000006)
#define REG_KEYINPUT (*(volatile uint16_t *)0x04000130)
#define PALETTE ((volatile uint16_t *)0x05000000)
#define VRAM ((volatile uint8_t *)0x06000000)
#define KEY_A 1
#define KEY_B 2
#define KEY_RIGHT 16
#define KEY_LEFT 32
#define KEY_UP 64
#define KEY_DOWN 128
#define KEY_R 256
#define KEY_L 512

typedef struct { const uint8_t *data; uint32_t size; } fixture_reader;
static uint16_t fixture_title_pixels[3][ARTWORK_MAX_PIXELS / 2]
    __attribute__((section(".ewram"), aligned(4)));
static uint16_t fixture_box_pixels[3][ARTWORK_MAX_PIXELS / 2]
    __attribute__((section(".ewram"), aligned(4)));
static artwork_info fixture_title_info[3], fixture_box_info[3];
static bool fixture_title_valid[3], fixture_box_valid[3];

static bool read_fixture(void *context, uint32_t offset, void *out, size_t size) {
  const fixture_reader *reader = context;
  if (!reader || offset > reader->size || size > reader->size - offset) return false;
  memcpy(out, reader->data + offset, size);
  return true;
}

static void load_fixtures(void) {
  for (unsigned i = 0; i < 3; ++i) {
    fixture_reader title = {browser_fixtures[i].title, sizeof(fixture_title_bmp_0)};
    fixture_reader box = {browser_fixtures[i].box, sizeof(fixture_box_bmp_0)};
    fixture_title_valid[i] = artwork_decode(read_fixture, &title, title.size,
        (uint8_t *)fixture_title_pixels[i], ARTWORK_MAX_PIXELS, &fixture_title_info[i]);
    fixture_box_valid[i] = artwork_decode(read_fixture, &box, box.size,
        (uint8_t *)fixture_box_pixels[i], ARTWORK_MAX_PIXELS, &fixture_box_info[i]);
  }
}

static int title_art_index_from_header(unsigned selected) {
  const uint8_t *rom_header = browser_fixtures[selected].rom;
  for (unsigned i = 0; i < 3; ++i)
    if (!memcmp(rom_header + 0xAC, browser_fixture_codes[i], 4)) return (int)i;
  return -1;
}

static void wait_vblank(void) {
  while (REG_VCOUNT >= 160) {}
  while (REG_VCOUNT < 160) {}
}

extern void dma_memset16(volatile void *, uint16_t, uint16_t);
static void fill_rect(volatile uint8_t *frame, int x, int y, int w, int h, uint8_t color) {
  if (x & 1) { --x; ++w; }
  if (w & 1) ++w;
  uint16_t pair = color | ((uint16_t)color << 8);
  if (x == 0 && w == 240) {
    dma_memset16(frame + y*240, pair, w*h/2);
    return;
  }
  for (int row = y; row < y + h; ++row) {
    volatile uint16_t *out = (volatile uint16_t *)(frame + row * 240 + x);
    dma_memset16(out, pair, w/2);
  }
}

static void card(volatile uint8_t *frame, const browser_carousel_item *item,
                 int vertical, int box_mode) {
  (void)vertical;
  uint8_t body = item->focused ? 2 : 3;
  fill_rect(frame, item->x, item->y, item->w, item->h, body);
  int code_index = title_art_index_from_header((unsigned)item->index);
  if (code_index < 0) return;
  unsigned art_index = (unsigned)code_index;
  const uint16_t *pixels = box_mode ? fixture_box_pixels[art_index] : fixture_title_pixels[art_index];
  const artwork_info *info = box_mode ? &fixture_box_info[art_index] : &fixture_title_info[art_index];
  bool valid = box_mode ? fixture_box_valid[art_index] : fixture_title_valid[art_index];
  if (!valid) return;
  unsigned x = item->x, y = item->y, width = item->w, height = item->h;
  if (box_mode) {
    width = height = item->w < item->h ? item->w : item->h;
    x += (item->w - width) / 2;
    y += (item->h - height) / 2;
  }
  if (x & 1) --x;
  if (item->focused && width == info->width && height == info->height)
    (void)artwork_blit_frame((volatile uint16_t *)frame, pixels, info, x, y);
  else
  (void)artwork_blit_scaled((volatile uint16_t *)frame, pixels, info, x, y, width, height);
}

static void render(volatile uint8_t *frame, int vertical, int selected, int offset, int box_mode) {
  fill_rect(frame, 0, 0, 240, 160, 0);
  fill_rect(frame, 0, 0, 240, 16, PREVIEW_THEME >= 0 ? 3 : 1);
  fill_rect(frame, 0, 144, 240, 16, PREVIEW_THEME >= 0 ? 3 : 1);
  if (PREVIEW_SCREEN == 2) {
    fill_rect(frame,0,144,240,16,0);
    render_ui_settings(frame); return;
  }
  if (PREVIEW_SCREEN == 1) { render_tools(frame); return; }
  browser_carousel_item items[3];
  unsigned n = browser_carousel_layout(vertical, selected, 3, offset, items);
  int focus = selected;
  for (unsigned i = 0; i < n; ++i) {
    if (!PREVIEW_ANIMATION) {
      unsigned index = items[i].index;
      const artwork_info *info = box_mode ? &fixture_box_info[index] : &fixture_title_info[index];
      const uint16_t *pixels = box_mode ? fixture_box_pixels[index] : fixture_title_pixels[index];
      int delta = (int)index-selected;
      int x = vertical ? (120-info->width)/2 : (240-info->width)/2+delta*(int)info->width;
      int y = vertical ? 48+delta*80 : 27;
      artwork_blit_cropped((volatile uint16_t *)frame,pixels,info,x,y);
    } else card(frame, &items[i], vertical, box_mode);
    if (items[i].focused) focus = items[i].index;
  }
  draw_text_ovf_rotate(browser_fixture_names[focus], frame,
      vertical ? (PREVIEW_ANIMATION ? 98 : 128) : 40, vertical ? 85 : 122, vertical ? (PREVIEW_ANIMATION ? 136 : 106) : 162, &text_animation);
}

int main(void) {
  PALETTE[0] = 0x18E3;  /* DSSMM-like dark blue-gray */
  PALETTE[1] = 0x7FFF;  /* light top/footer */
  PALETTE[2] = 0x3E9F;  /* selected blue */
  PALETTE[3] = 0x56B5;  /* side card */
  for (unsigned i = 0; i < ARTWORK_PALETTE_SIZE; ++i)
    PALETTE[ARTWORK_PALETTE_BASE + i] = artwork_palette_color(i);
  if (PREVIEW_THEME >= 0 && PREVIEW_THEME < THEME_COUNT) {
    const t_theme_colors *colors = &themes[PREVIEW_THEME];
    PALETTE[0]=colors->bg_color; PALETTE[1]=colors->ft_color;
    PALETTE[2]=colors->hi_color; PALETTE[3]=colors->fg_color;
    menu_theme=PREVIEW_THEME;
  }
  load_fixtures();
  preview_select_ui(PREVIEW_OPTION);
  smenu.tools.selector = PREVIEW_OPTION < ToolsMAX ? PREVIEW_OPTION : ToolsMAX-1;
  REG_DISPCNT = 4 | 0x0400; /* Mode 4, BG2 enabled, framebuffer page 0 */
  int selected = 1, offset = 0, vertical = PREVIEW_VERTICAL, box_mode = PREVIEW_BOX;
  unsigned old_keys = 0, back_page = 1, demo_frame = 0;
  for (;;) {
    unsigned keys = (unsigned)(~REG_KEYINPUT) & 0x03ff;
    unsigned pressed = keys & ~old_keys;
    old_keys = keys;
    if (pressed & KEY_LEFT) { if (selected > 0) { --selected; offset = browser_motion_retarget(offset, -1); } }
    if (pressed & KEY_RIGHT) { if (selected < 2) { ++selected; offset = browser_motion_retarget(offset, 1); } }
    if (pressed & KEY_UP) { if (selected > 0) { --selected; offset = browser_motion_retarget(offset, -1); } }
    if (pressed & KEY_DOWN) { if (selected < 2) { ++selected; offset = browser_motion_retarget(offset, 1); } }
    if (pressed & KEY_A) vertical = !vertical;
    if (pressed & KEY_B) { selected = 1; offset = 0; }
    if (pressed & KEY_L) box_mode = 0;
    if (pressed & KEY_R) box_mode = 1;
    ++text_animation;
    if (PREVIEW_AUTOMOVE && ++demo_frame % 120 == 0) {
      int next = selected == 1 ? 0 : 1;
      offset = browser_motion_retarget(offset, next - selected);
      selected = next;
      text_animation = 0;
    }
    offset = !PREVIEW_ANIMATION ? 0 : PREVIEW_OFFSET ? PREVIEW_OFFSET : browser_motion_advance(offset, 1);
    volatile uint8_t *back = VRAM + (back_page ? 0xA000 : 0);
    volatile uint16_t *timers = (volatile uint16_t *)0x04000100;
    if (PREVIEW_TIMING) {
      timers[1] = 0; timers[3] = 0;
      timers[0] = 0; timers[2] = 0;
      timers[3] = 0x0084; /* Timer1 cascades Timer0 overflow. */
      timers[1] = 0x0080; /* CPU-clock ticks. */
    }
    render(back, vertical, selected, offset, box_mode);
    if (PREVIEW_TIMING) {
      timers[1] = 0; timers[3] = 0;
      uint32_t cycles = timers[0] | ((uint32_t)timers[2] << 16);
      char timing[48];
      snprintf(timing, sizeof(timing), "Draw: %lu cycles", (unsigned long)cycles);
      draw_text_idx8_bus16(timing, (uint8_t *)back + 144*240 + 8, 240, 0);
    }
    wait_vblank();
    REG_DISPCNT = 4 | 0x0400 | (back_page ? 0x0010 : 0);
    back_page ^= 1;
  }
}
