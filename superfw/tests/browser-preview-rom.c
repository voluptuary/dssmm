#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "browser_carousel.h"
#include "browser_motion.h"
#include "artwork.h"
#include "browser_fixture_assets.h"
#include "fonts/font_render.h"
#include "browser_preview_text.h"
#include "ingame_preview.h"
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

static bool preview_load_art(const char *path,unsigned mode,uint8_t *pixels,size_t cap,uint8_t *bmp,size_t bcap,artwork_info *info) {
  (void)path;(void)bmp;(void)bcap;
  const artwork_info *source=mode?&fixture_box_info[1]:&fixture_title_info[1];
  if(cap<(size_t)source->width*source->height)return false;
  memcpy(pixels,mode?fixture_box_pixels[1]:fixture_title_pixels[1],(size_t)source->width*source->height);
  *info=*source;return true;
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
  (void)body; /* The full browser background is already clear. */
  int code_index = title_art_index_from_header((unsigned)item->index);
  if (code_index < 0) return;
  unsigned art_index = (unsigned)code_index;
  const uint16_t *pixels = box_mode ? fixture_box_pixels[art_index] : fixture_title_pixels[art_index];
  const artwork_info *info = box_mode ? &fixture_box_info[art_index] : &fixture_title_info[art_index];
  bool valid = box_mode ? fixture_box_valid[art_index] : fixture_title_valid[art_index];
  if (!valid) return;
  int x = item->x, y = item->y;
  unsigned width = item->w, height = item->h;
  if (box_mode) {
    width = height = item->w < item->h ? item->w : item->h;
    x += (item->w - width) / 2;
    y += (item->h - height) / 2;
  }
  if (x & 1) --x;
  (void)artwork_blit_scaled_cropped((volatile uint16_t *)frame,pixels,info,x,y,width,height);
}

static void reload_theme(unsigned theme) {
  font_set_style(menu_font);
  const t_theme_colors colors=menu_theme_colors(theme,menu_accent);
  PALETTE[0]=colors.bg_color;PALETTE[1]=colors.ft_color;
  PALETTE[2]=colors.hi_color;PALETTE[3]=colors.fg_color;
  PALETTE[16]=colors.fg_color;PALETTE[17]=colors.bg_color;PALETTE[18]=colors.ft_color;PALETTE[19]=colors.hi_color;
}

static void render(volatile uint8_t *frame, int vertical, int selected, int offset, int box_mode) {
  fill_rect(frame, 0, 0, 240, 160, 0);
  if(PREVIEW_SCREEN==13 || PREVIEW_SCREEN==14) {
    dssmm_background_palette(PALETTE,menu_theme==THEME_DSSMM_DARK);
    if(launcher_page==1)render_start(frame);
    else {
      dssmm_background_draw(frame,menu_theme==THEME_DSSMM_DARK,false);
      if(launcher_page==2){draw_central_text("Settings",frame,120,0);render_settings_home(frame);}
      else if(smenu.menu_tab==MENUTAB_UILANG)render_ui_settings(frame);
      else if(smenu.menu_tab==MENUTAB_SETTINGS)render_settings(frame);
      else if(smenu.menu_tab==MENUTAB_TOOLS)render_tools(frame);
      else draw_central_text("SD Card",frame,120,72);
    }
    return;
  }
  fill_rect(frame, 0, 0, 240, 16, PREVIEW_THEME >= 0 ? 3 : 1);
  fill_rect(frame, 0, 144, 240, 16, PREVIEW_THEME >= 0 ? 3 : 1);
  if (PREVIEW_SCREEN >= 7 && PREVIEW_SCREEN <= 12) {
    fill_rect(frame,0,0,240,160,0); /* In-game has no launcher tab/footer bars. */
    copt=PREVIEW_OPTION < (PREVIEW_SCREEN==7 ? 6u : PREVIEW_SCREEN==10 ? 7u : 4u) ? PREVIEW_OPTION : 0;
    if(PREVIEW_SCREEN==7)igm_draw_main_menu((uint8_t *)frame,0);
    if(PREVIEW_SCREEN==8)igm_draw_reset_menu((uint8_t *)frame,0);
    if(PREVIEW_SCREEN==9)igm_draw_save_menu((uint8_t *)frame,0);
    if(PREVIEW_SCREEN==10)igm_draw_rtc_menu((uint8_t *)frame,0);
    if(PREVIEW_SCREEN==11){igm_fake_cheats();copt=PREVIEW_OPTION<2?PREVIEW_OPTION:0;igm_draw_cheats_menu((uint8_t *)frame,0);}
    if(PREVIEW_SCREEN==12){copt=PREVIEW_OPTION<3?PREVIEW_OPTION:0;igm_draw_states_menu((uint8_t *)frame,0);}
    return;
  }
  if (PREVIEW_SCREEN >= 4 && PREVIEW_SCREEN <= 6) {
    draw_box_outline(frame,2,238,18,158,FG_COLOR);
    draw_text_ovf("⯇",frame,10,23,8);
    draw_text_ovf("⯈",frame,222,23,8);
  }
  if (PREVIEW_SCREEN == 5) {
    const t_load_gba_info info={0,true,true,true};
    (void)render_gbarom_patching(frame,&info,PREVIEW_OPTION<5 ? PREVIEW_OPTION : 4);
    return;
  }
  if (PREVIEW_SCREEN == 6) {
    const t_load_gba_lcfg info={0,0,0,true,true};
    (void)render_gbarom_loading(frame,&info,true,PREVIEW_OPTION<5 ? PREVIEW_OPTION : 4);
    return;
  }
  if (PREVIEW_SCREEN == 4) {
    render_gbarom_info(frame, PREVIEW_OPTION==6 ? "/Game Boy Advance/P/A very long synthetic title for scrolling checks (USA).gba" : browser_fixture_names[1], false, "FAKE", 0, PREVIEW_OPTION<6 ? PREVIEW_OPTION : -1);
    draw_central_text(msgs[lang_id][MSG_LOAD_GBA],frame,120,134);
    return;
  }
  if (PREVIEW_SCREEN == 3) { render_settings(frame); return; }
  if (PREVIEW_SCREEN == 2) {
    fill_rect(frame,0,144,240,16,0);
    render_ui_settings(frame);
    return;
  }
  if (PREVIEW_SCREEN == 1) { render_tools(frame); return; }
  browser_carousel_item items[3];
  unsigned n = browser_carousel_layout(vertical, selected, 3, offset, items);
  int focus = selected;
  for (unsigned i = 0; i < n; ++i) {
    card(frame, &items[i], vertical, box_mode);
    if (items[i].focused) focus = items[i].index;
  }
  draw_text_ovf_rotate(browser_fixture_names[focus], frame,
      vertical ? 98 : 40, vertical ? 85 : 122, vertical ? 136 : 162, &text_animation);
}

int main(void) {
  menu_font=PREVIEW_FONT;
  font_set_style(menu_font);
  PALETTE[0] = 0x18E3;  /* DSSMM-like dark blue-gray */
  PALETTE[1] = 0x7FFF;  /* light top/footer */
  PALETTE[2] = 0x3E9F;  /* selected blue */
  PALETTE[3] = 0x56B5;  /* side card */
  PALETTE[16]=PALETTE[3];PALETTE[17]=PALETTE[0];PALETTE[18]=PALETTE[1];PALETTE[19]=PALETTE[2];
  for (unsigned i = 0; i < ARTWORK_PALETTE_SIZE; ++i)
    PALETTE[ARTWORK_PALETTE_BASE + i] = artwork_palette_color(i);
  if (PREVIEW_THEME >= 0 && PREVIEW_THEME < THEME_COUNT) {
    const t_theme_colors *colors = &themes[PREVIEW_THEME];
    PALETTE[0]=colors->bg_color; PALETTE[1]=colors->ft_color;
    PALETTE[2]=colors->hi_color; PALETTE[3]=colors->fg_color;
    PALETTE[16]=colors->fg_color;PALETTE[17]=colors->bg_color;PALETTE[18]=colors->ft_color;PALETTE[19]=colors->hi_color;
    menu_theme=PREVIEW_THEME;
  }
  if (PREVIEW_SCREEN==12) {
    for(unsigned i=0;i<16;++i)PALETTE[128+i]=menu_icons_pal[i];
    PALETTE[128]=PALETTE[0];
  }
  load_fixtures();
  launcher_page=PREVIEW_SCREEN==14?2:1;
  launcher_selected=PREVIEW_OPTION<4?PREVIEW_OPTION:1;
  launcher_category=PREVIEW_OPTION<5?PREVIEW_OPTION:0;
  smenu.recent.maxentries=1;
  strcpy(preview_sdram.rentries[0].fpath,browser_fixture_names[1]);
  preview_select_ui(PREVIEW_OPTION);
  smenu.set.selector = PREVIEW_OPTION < SettMAX ? PREVIEW_OPTION : SettSave;
  smenu.tools.selector = PREVIEW_OPTION < ToolsMAX ? PREVIEW_OPTION : ToolsMAX-1;
  REG_DISPCNT = 4 | 0x0400; /* Mode 4, BG2 enabled, framebuffer page 0 */
  int selected = 1, offset = 0, vertical = PREVIEW_VERTICAL, box_mode = PREVIEW_BOX;
  unsigned old_keys = 0, back_page = 1, demo_frame = 0;
  for (;;) {
    unsigned keys = (unsigned)(~REG_KEYINPUT) & 0x03ff;
    unsigned pressed = keys & ~old_keys;
    old_keys = keys;
    if(PREVIEW_SCREEN==13 || PREVIEW_SCREEN==14) {
      unsigned newkeys=pressed;
      if(newkeys)preview_launcher_keypress(newkeys);
    }
    if(PREVIEW_SCREEN==2) {
      if(pressed)keypress_menu_uisettings(pressed);
    } else {
    if (pressed & KEY_LEFT) { if (selected > 0) { --selected; offset = browser_motion_retarget(offset, -1); } }
    if (pressed & KEY_RIGHT) { if (selected < 2) { ++selected; offset = browser_motion_retarget(offset, 1); } }
    if (pressed & KEY_UP) { if (selected > 0) { --selected; offset = browser_motion_retarget(offset, -1); } }
    if (pressed & KEY_DOWN) { if (selected < 2) { ++selected; offset = browser_motion_retarget(offset, 1); } }
    if (pressed & KEY_A) vertical = !vertical;
    if (pressed & KEY_B) { selected = 1; offset = 0; }
    if (pressed & KEY_L) box_mode = 0;
    if (pressed & KEY_R) box_mode = 1;
    }
    ++text_animation;
    if (PREVIEW_AUTOMOVE && ++demo_frame % 120 == 0) {
      int next = selected == 1 ? 0 : 1;
      offset = browser_motion_retarget(offset, next - selected);
      selected = next;
      text_animation = 0;
    }
    offset = PREVIEW_OFFSET ? PREVIEW_OFFSET : browser_motion_advance(offset, 1);
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
